#!/usr/bin/env python3
"""
slice_spritesheet.py — Convert the Gemini bull sprite sheet into
per-state GIFs ready to flash to the XIAO ESP32-S3 Buddy device.

Usage:
    python3 slice_spritesheet.py <sprite_sheet.png> [output_dir]

Output:
    output_dir/characters/bullhorn/
        manifest.json
        sleep.gif
        idle.gif
        busy.gif
        attention.gif
        celebrate.gif
        dizzy.gif
        heart.gif

Then flash with:
    pio run --target uploadfs
"""

import sys, os, json
import numpy as np
from PIL import Image

# ── Config ──────────────────────────────────────────────────────────────────

# States in the order they appear top-to-bottom in the sprite sheet
# (matches what Gemini generated)
STATE_ORDER = ["sleep", "idle", "busy", "attention", "celebrate", "dizzy", "heart"]

# Frame delay in ms per state (tune to taste)
FRAME_DELAYS = {
    "sleep":     300,
    "idle":      300,
    "busy":      300,
    "attention": 300,
    "celebrate": 300,
    "dizzy":     300,
    "heart":     300,
}

GIF_SIZE    = 80    # output canvas px (square) — renders at 1.5× = 120px on device
BG_COLOR    = (0, 0, 0, 0)   # transparent

# ── Sprite detection ─────────────────────────────────────────────────────────

def remove_white_bg(img_rgba, threshold=230):
    """Turn near-white pixels transparent; keep orange."""
    arr = np.array(img_rgba, dtype=np.uint8)
    r, g, b, a = arr[...,0], arr[...,1], arr[...,2], arr[...,3]
    white = (r > threshold) & (g > threshold) & (b > threshold)
    arr[white, 3] = 0
    return Image.fromarray(arr, "RGBA")

def find_sprite_rows(alpha, row_gap=8):
    """
    Given the alpha channel as a 2-D numpy array, return a list of
    (y_start, y_end) row bands that contain orange pixels, with bands
    separated by at least row_gap empty rows.
    """
    occupied = alpha.sum(axis=1) > 0   # True for rows that have any content
    bands = []
    in_band = False
    start = 0
    for y, occ in enumerate(occupied):
        if occ and not in_band:
            in_band = True
            start = y
        elif not occ and in_band:
            # Check if this is just a small gap (within a row)
            # Look ahead for row_gap empty rows before closing the band
            gap = 0
            j = y
            while j < len(occupied) and not occupied[j]:
                gap += 1
                j += 1
            if gap >= row_gap:
                bands.append((start, y - 1))
                in_band = False
    if in_band:
        bands.append((start, len(occupied) - 1))
    return bands

def find_sprites_in_row(alpha_row, col_gap=4):
    """
    Given a 2-D alpha slice for one row, return list of (x_start, x_end)
    bounding boxes of individual sprites in that row.
    """
    occupied = alpha_row.sum(axis=0) > 0
    sprites = []
    in_sprite = False
    start = 0
    for x, occ in enumerate(occupied):
        if occ and not in_sprite:
            in_sprite = True
            start = x
        elif not occ and in_sprite:
            gap = 0
            j = x
            while j < len(occupied) and not occupied[j]:
                gap += 1
                j += 1
            if gap >= col_gap:
                sprites.append((start, x - 1))
                in_sprite = False
    if in_sprite:
        sprites.append((start, len(occupied) - 1))
    return sprites

# ── Frame extraction ─────────────────────────────────────────────────────────

def extract_frame(img_rgba, x0, y0, x1, y1, target_size, global_scale=None,
                  bottom_pad=4, clamp_x0=None, clamp_x1=None):
    """Crop sprite, scale uniformly, place on canvas.

    Horizontal: centred.
    Vertical:   bottom-aligned with `bottom_pad` pixels of breathing room.
                This keeps the "ground line" consistent across all frames of
                an animation — a leaping pose won't float above a standing one.

    global_scale: fixed scale applied to every frame so all states appear the
                  same apparent size (computed once by compute_global_scale).
    bottom_pad:   pixels of space between the sprite bottom and canvas bottom.
    clamp_x0/x1: hard limits for x expansion (used in grid mode to prevent
                  bleeding into neighbouring cells).
    """
    margin = 6
    x0 = max(0, x0 - margin)
    y0 = max(0, y0 - margin)
    x1 = min(img_rgba.width - 1,  x1 + margin)
    y1 = min(img_rgba.height - 1, y1 + margin)
    # Clamp x to cell boundaries if provided
    if clamp_x0 is not None: x0 = max(x0, clamp_x0)
    if clamp_x1 is not None: x1 = min(x1, clamp_x1)

    crop = img_rgba.crop((x0, y0, x1+1, y1+1))
    w, h = crop.size

    if global_scale is not None:
        scale = global_scale
    else:
        scale = min(target_size / w, target_size / h)

    new_w = max(1, int(w * scale))
    new_h = max(1, int(h * scale))
    new_w = min(new_w, target_size)
    new_h = min(new_h, target_size)
    crop = crop.resize((new_w, new_h), Image.LANCZOS)

    canvas = Image.new("RGBA", (target_size, target_size), BG_COLOR)
    paste_x = (target_size - new_w) // 2   # centred horizontally
    paste_y = (target_size - new_h) // 2   # centred vertically
    canvas.paste(crop, (paste_x, paste_y), crop.split()[3])
    return canvas


def compute_global_scale(img_rgba, alpha, row_states, target_size):
    """One pass over all sprites to find the scale factor that fits the
    largest sprite within target_size.  Using this for every frame keeps
    the character the same apparent size across all animation states.
    """
    margin = 6
    max_w = 1
    max_h = 1
    for _state, ry0, ry1 in row_states:
        alpha_row = alpha[ry0:ry1+1, :]
        col_boxes = find_sprites_in_row(alpha_row, col_gap=6)
        for (cx0, cx1) in col_boxes:
            col_alpha = alpha[ry0:ry1+1, cx0:cx1+1]
            ys = np.where(col_alpha.sum(axis=1) > 0)[0]
            if len(ys) == 0:
                continue
            local_y0, local_y1 = ys[0], ys[-1]
            # Mimic the margin expansion in extract_frame
            fx0 = max(0, cx0 - margin)
            fy0 = max(0, ry0 + local_y0 - margin)
            fx1 = min(img_rgba.width  - 1, cx1 + margin)
            fy1 = min(img_rgba.height - 1, ry0 + local_y1 + margin)
            w = fx1 - fx0 + 1
            h = fy1 - fy0 + 1
            if w > max_w: max_w = w
            if h > max_h: max_h = h
    scale = min(target_size / max_w, target_size / max_h)
    print(f"  global scale: {scale:.3f}  (largest sprite: {max_w}×{max_h} → fits in {target_size}px)")
    return scale

# ── Palette conversion (2-color + transparency for minimal GIF size) ─────────

def to_indexed(frame_rgba, n_colors=64):
    """Convert RGBA frame → indexed P-mode image for animated GIF.

    Uses PIL's median-cut quantizer so anti-aliased edges are preserved
    (smooth gradients instead of blocky 5-colour outlines).
    Transparent pixels (alpha < 60) are always mapped to palette index 0.
    """
    arr = np.array(frame_rgba)
    alpha_arr = arr[..., 3]
    # Treat any pixel with alpha < 128 as transparent — catches anti-aliased edges
    transparent_mask = alpha_arr < 128

    # Pre-multiply alpha so semi-transparent edge pixels blend to black
    # rather than showing a sentinel tint.
    rgb_arr = arr[..., :3].astype(np.float32)
    a_norm  = (alpha_arr / 255.0)[..., np.newaxis]
    blended = (rgb_arr * a_norm).astype(np.uint8)
    rgb = Image.fromarray(blended, "RGB")

    # Median-cut quantisation — no dithering (looks noisy on small sprites)
    q = rgb.quantize(colors=n_colors, method=Image.Quantize.MEDIANCUT, dither=0)
    pal = q.getpalette()   # flat list: 256 × [R, G, B]

    # Find palette index closest to black (our blended background)
    best_idx, best_dist = 0, float("inf")
    for i in range(n_colors):
        r, g, b = pal[i*3], pal[i*3+1], pal[i*3+2]
        d = r*r + g*g + b*b
        if d < best_dist:
            best_dist = d
            best_idx  = i

    # Swap black index → 0 in both palette and pixel data
    if best_idx != 0:
        pal[0:3], pal[best_idx*3:best_idx*3+3] = \
            list(pal[best_idx*3:best_idx*3+3]), list(pal[0:3])
        px = np.array(q, dtype=np.uint8)
        tmp = px == 0
        px[px == best_idx] = 0
        px[tmp]            = best_idx
        q = Image.fromarray(px, "P")

    q.putpalette(pal)

    # Force all transparent pixels to index 0
    px = np.array(q, dtype=np.uint8)
    px[transparent_mask] = 0
    out = Image.fromarray(px, "P")
    out.putpalette(pal)
    return out

# ── Manifest writer ──────────────────────────────────────────────────────────

def _write_manifest(out_base, gif_state_map):
    mpath = os.path.join(out_base, "manifest.json")
    # Merge into existing manifest so multiple single-row runs don't clobber each other
    manifest = {
        "name": "Bullhorn",
        "mode": "gif",
        "colors": {
            "body":    "#F26522",
            "bg":      "#000000",
            "text":    "#FFFFFF",
            "textDim": "#886644",
            "ink":     "#F26522"
        },
        "states": {}
    }
    if os.path.exists(mpath):
        with open(mpath) as f:
            try:
                existing = json.load(f)
                manifest["states"] = existing.get("states", {})
            except json.JSONDecodeError:
                pass
    manifest["states"].update(gif_state_map)
    with open(mpath, "w") as f:
        json.dump(manifest, f, indent=2)
    total = sum(os.path.getsize(os.path.join(out_base, fn))
                for fn in os.listdir(out_base) if fn.endswith((".gif",".json")))
    print(f"\nDone! → {out_base}")
    print(f"Total: {total//1024} KB  (LittleFS budget ~800 KB)")


# ── Main ─────────────────────────────────────────────────────────────────────

def save_debug_image(img_rgba, alpha, row_states, out_path):
    """Draw row bands (blue) and per-frame bounding boxes (green/yellow) on the
    original image and save as debug_layout.png so you can see exactly what the
    slicer detected."""
    from PIL import ImageDraw, ImageFont
    debug = img_rgba.copy().convert("RGB")
    draw  = ImageDraw.Draw(debug)

    COLORS = ["#00FF00","#FFFF00","#00FFFF","#FF00FF","#FF8800","#88FF00","#00FFCC"]

    for si, (state, ry0, ry1) in enumerate(row_states):
        col = COLORS[si % len(COLORS)]
        # Row band — left/right thin lines + label
        draw.rectangle([0, ry0, img_rgba.width - 1, ry1], outline="#4444FF", width=1)
        draw.text((2, ry0 + 2), state, fill="#4444FF")

        alpha_row = alpha[ry0:ry1+1, :]
        col_boxes = find_sprites_in_row(alpha_row, col_gap=6)
        for fi, (cx0, cx1) in enumerate(col_boxes):
            col_alpha = alpha[ry0:ry1+1, cx0:cx1+1]
            ys = np.where(col_alpha.sum(axis=1) > 0)[0]
            if len(ys) == 0:
                continue
            local_y0, local_y1 = ys[0], ys[-1]
            ax0 = cx0
            ay0 = ry0 + local_y0
            ax1 = cx1
            ay1 = ry0 + local_y1
            draw.rectangle([ax0, ay0, ax1, ay1], outline=col, width=2)
            draw.text((ax0 + 2, ay0 + 2), str(fi), fill=col)

    debug.save(out_path)
    print(f"  Debug layout → {out_path}")


def main():
    debug_only   = "--debug"       in sys.argv
    save_crops   = "--save-crops"  in sys.argv
    use_grid     = "--grid"        in sys.argv
    crop_bottom  = 0
    row_gap      = 10
    col_gap      = 6
    grid_rows    = len(STATE_ORDER)
    grid_cols    = 4
    grid_pad     = 0
    cell_w       = None   # if set, use exact cell size instead of dividing equally
    cell_h       = None
    offset_x     = 0      # pixels before first column
    offset_y     = 0      # pixels before first row
    state_offset = 0      # which STATE_ORDER index to start from
    for a in sys.argv[1:]:
        if   a.startswith("--crop-bottom="): crop_bottom  = int(a.split("=",1)[1])
        elif a.startswith("--row-gap="):     row_gap      = int(a.split("=",1)[1])
        elif a.startswith("--col-gap="):     col_gap      = int(a.split("=",1)[1])
        elif a.startswith("--grid-rows="):   grid_rows    = int(a.split("=",1)[1])
        elif a.startswith("--grid-cols="):   grid_cols    = int(a.split("=",1)[1])
        elif a.startswith("--grid-pad="):    grid_pad     = int(a.split("=",1)[1])
        elif a.startswith("--cell-w="):      cell_w       = int(a.split("=",1)[1])
        elif a.startswith("--cell-h="):      cell_h       = int(a.split("=",1)[1])
        elif a.startswith("--offset-x="):    offset_x     = int(a.split("=",1)[1])
        elif a.startswith("--offset-y="):    offset_y     = int(a.split("=",1)[1])
        elif a.startswith("--state-offset="):state_offset = int(a.split("=",1)[1])
    args = [a for a in sys.argv[1:] if not a.startswith("--")]

    if len(args) < 1:
        print("Usage: python3 slice_spritesheet.py <sprite.png> [output_dir]")
        print("  --debug           save debug_layout.png, no GIFs written")
        print("  --save-crops      save raw RGBA frames to debug_crops/")
        print("  --grid            divide image into equal cells (reliable for structured sheets)")
        print("  --grid-rows=N     rows in grid mode (default 7)")
        print("  --grid-cols=N     cols in grid mode (default 4)")
        print("  --grid-pad=N      pixels to trim from each cell edge (default 0)")
        print("  --crop-bottom=N   remove N pixels from the bottom (removes watermarks)")
        print("  --row-gap=N       alpha-detection row gap (default 10)")
        print("  --col-gap=N       alpha-detection col gap (default 6)")
        sys.exit(1)

    sprite_path = args[0]
    out_base    = args[1] if len(args) > 1 else os.path.join(
        os.path.dirname(sprite_path), "characters", "bullhorn")

    print(f"Loading: {sprite_path}")
    img = Image.open(sprite_path).convert("RGBA")
    print(f"  Size: {img.size}")

    if crop_bottom > 0:
        w, h = img.size
        img = img.crop((0, 0, w, h - crop_bottom))
        print(f"  Cropped bottom {crop_bottom}px → new size: {img.size}")

    img = remove_white_bg(img)
    alpha = np.array(img)[..., 3]

    # Detect row bands
    row_bands = find_sprite_rows(alpha, row_gap=row_gap)
    print(f"  Found {len(row_bands)} row band(s)  (row_gap={row_gap})")

    if len(row_bands) != len(STATE_ORDER):
        print(f"  WARNING: expected {len(STATE_ORDER)} rows (one per state), "
              f"got {len(row_bands)}. Try --row-gap=N (currently {row_gap}).")

    # Assign states to rows (top → bottom)
    row_states = []
    for i, (y0, y1) in enumerate(row_bands):
        state = STATE_ORDER[i] if i < len(STATE_ORDER) else f"state{i}"
        row_states.append((state, y0, y1))

    # Print per-band stats; flag bands that are much taller than the median
    heights = [y1 - y0 for _, y0, y1 in row_states]
    if heights:
        median_h = sorted(heights)[len(heights) // 2]
    for state, ry0, ry1 in row_states:
        alpha_row = alpha[ry0:ry1+1, :]
        col_boxes = find_sprites_in_row(alpha_row, col_gap=col_gap)
        h = ry1 - ry0
        flag = "  *** TALL — may have merged rows, try larger --row-gap" \
               if h > median_h * 1.5 else ""
        print(f"  {state:12s}: {len(col_boxes)} frames  y={ry0}..{ry1} h={h}{flag}")

    if debug_only:
        debug_path = os.path.join(os.path.dirname(sprite_path), "debug_layout.png")
        save_debug_image(img, alpha, row_states, debug_path)
        print("Debug mode — no GIFs written.")
        return

    os.makedirs(out_base, exist_ok=True)

    crops_dir = os.path.join(os.path.dirname(sprite_path), "debug_crops")
    if save_crops:
        os.makedirs(crops_dir, exist_ok=True)
        print(f"  Saving raw RGBA crops → {crops_dir}/")

    # ── Grid mode: divide image into equal cells ───────────────────────────────
    # More reliable than alpha-detection when the sheet has a regular layout.
    # Use --grid to activate; tune with --grid-pad if cells have whitespace.
    if use_grid:
        IW, IH = img.size
        if cell_w is None: cell_w = (IW - offset_x) // grid_cols
        if cell_h is None: cell_h = (IH - offset_y) // grid_rows
        print(f"  Grid mode: {grid_rows}×{grid_cols} cells  "
              f"cell={cell_w}×{cell_h}px  offset={offset_x},{offset_y}  pad={grid_pad}px")

        # Compute scale per-row so a tall celebrate row (stars floating above
        # the face) doesn't shrink every other state's faces.
        # First pass: find the largest content bbox per row.
        row_scales = []
        for ri in range(grid_rows):
            max_w, max_h = 1, 1
            for ci in range(grid_cols):
                x0 = offset_x + ci * cell_w + grid_pad
                y0 = offset_y + ri * cell_h + grid_pad
                x1 = x0 + cell_w - grid_pad * 2
                y1 = y0 + cell_h - grid_pad * 2
                cell_alpha = alpha[y0:y1, x0:x1]
                ys = np.where(cell_alpha.sum(axis=1) > 0)[0]
                xs = np.where(cell_alpha.sum(axis=0) > 0)[0]
                if len(ys) == 0 or len(xs) == 0:
                    continue
                w = int(xs[-1] - xs[0]) + 1
                h = int(ys[-1] - ys[0]) + 1
                if w > max_w: max_w = w
                if h > max_h: max_h = h
            s = min(GIF_SIZE / max_w, GIF_SIZE / max_h) * 0.92
            row_scales.append(s)
            state_name = STATE_ORDER[state_offset + ri] if (state_offset + ri) < len(STATE_ORDER) else str(ri)
            print(f"  row {ri} ({state_name:12s}) scale={s:.3f}  "
                  f"largest content: {max_w}×{max_h}px")

        gif_state_map = {}
        for ri in range(min(grid_rows, len(STATE_ORDER) - state_offset)):
            state      = STATE_ORDER[state_offset + ri]
            row_scale  = row_scales[ri]
            frames = []

            # First pass: compute the UNION bbox across all frames in this row
            # so that floating elements (Z letters, stars) don't cause the
            # character to jump position between frames.
            row_fy0, row_fy1 = 999999, 0
            row_fx0, row_fx1 = 999999, 0
            for ci in range(grid_cols):
                x0 = offset_x + ci * cell_w + grid_pad
                y0 = offset_y + ri * cell_h + grid_pad
                x1 = x0 + cell_w - grid_pad * 2
                y1 = y0 + cell_h - grid_pad * 2
                cell_alpha = alpha[y0:y1, x0:x1]
                ys = np.where(cell_alpha.sum(axis=1) > 0)[0]
                if len(ys) == 0:
                    continue
                row_fy0 = min(row_fy0, y0 + int(ys[0]))
                row_fy1 = max(row_fy1, y0 + int(ys[-1]))

            for ci in range(grid_cols):
                x0 = offset_x + ci * cell_w + grid_pad
                y0 = offset_y + ri * cell_h + grid_pad
                x1 = x0 + cell_w - grid_pad * 2
                y1 = y0 + cell_h - grid_pad * 2
                cell_alpha = alpha[y0:y1, x0:x1]
                ys = np.where(cell_alpha.sum(axis=1) > 0)[0]
                if len(ys) == 0:
                    continue
                # Use strict cell boundaries for x (no content detection) so
                # decorations near cell edges don't bleed into neighbouring frames.
                # Use shared vertical bounds so character doesn't jump between frames.
                fx0, fx1 = x0, x1
                fy0, fy1 = row_fy0, row_fy1
                frame = extract_frame(img, fx0, fy0, fx1, fy1,
                                      GIF_SIZE, global_scale=row_scale,
                                      clamp_x0=x0, clamp_x1=x1)
                if save_crops:
                    cname = f"{state}_f{ci}_src_{fx0}x{fy0}-{fx1}x{fy1}.png"
                    frame.save(os.path.join(crops_dir, cname))
                frames.append(to_indexed(frame))

            print(f"  {state:12s}: {len(frames)} frames")
            if not frames:
                continue

            fname = f"{state}.gif"
            fpath = os.path.join(out_base, fname)
            delay = FRAME_DELAYS.get(state, 200)
            frames[0].save(fpath, format="GIF", save_all=True,
                           append_images=frames[1:], loop=0, duration=delay,
                           transparency=0, disposal=2, optimize=False)
            print(f"    → {fname}  ({os.path.getsize(fpath)//1024} KB)")
            gif_state_map[state] = fname

        # manifest written below — skip the alpha-detection path
        _write_manifest(out_base, gif_state_map)
        return

    # ── Alpha-detection path (original, for irregular sheets) ─────────────────
    # Compute one scale that fits the *largest* sprite across all states.
    global_scale = compute_global_scale(img, alpha, row_states, GIF_SIZE)

    gif_state_map = {}

    for state, ry0, ry1 in row_states:
        alpha_row = alpha[ry0:ry1+1, :]
        col_boxes = find_sprites_in_row(alpha_row, col_gap=col_gap)
        print(f"  {state:12s}: {len(col_boxes)} frames")

        frames = []
        for (cx0, cx1) in col_boxes:
            # Find tight vertical bounds within this column slice
            col_alpha = alpha[ry0:ry1+1, cx0:cx1+1]
            ys = np.where(col_alpha.sum(axis=1) > 0)[0]
            if len(ys) == 0:
                continue
            local_y0, local_y1 = ys[0], ys[-1]
            frame = extract_frame(img,
                                  cx0, ry0 + local_y0,
                                  cx1, ry0 + local_y1,
                                  GIF_SIZE,
                                  global_scale=global_scale)
            if save_crops:
                crop_name = f"{state}_f{len(frames)}_src_{cx0}x{ry0+local_y0}-{cx1}x{ry0+local_y1}.png"
                frame.save(os.path.join(crops_dir, crop_name))
            frames.append(to_indexed(frame))

        if not frames:
            print(f"    (no frames found, skipping)")
            continue

        # Save GIF
        fname = f"{state}.gif"
        fpath = os.path.join(out_base, fname)
        delay = FRAME_DELAYS.get(state, 200)

        frames[0].save(
            fpath,
            format="GIF",
            save_all=True,
            append_images=frames[1:],
            loop=0,
            duration=delay,
            transparency=0,
            disposal=2,
            optimize=False,
        )
        sz = os.path.getsize(fpath)
        print(f"    → {fname}  ({sz//1024} KB)")
        gif_state_map[state] = fname

    _write_manifest(out_base, gif_state_map)

if __name__ == "__main__":
    main()
