#!/usr/bin/env python3
"""
replace_row.py — Paste a replacement row strip into a sprite sheet.

Usage:
    python3 replace_row.py <sheet.png> <new_row.png> <row_index> [output.png]

    row_index   0-based row number to replace (0=sleep, 1=idle, 2=busy,
                3=attention, 4=celebrate, 5=dizzy, 6=heart)

Example — replace the attention row (index 3):
    python3 tools/replace_row.py ~/Downloads/bull_sprites.png \
                                 ~/Downloads/attention_row.png \
                                 3
"""

import sys
from PIL import Image

STATE_NAMES = ["sleep", "idle", "busy", "attention", "celebrate", "dizzy", "heart"]
N_ROWS = len(STATE_NAMES)

def main():
    if len(sys.argv) < 4:
        print(__doc__)
        sys.exit(1)

    sheet_path   = sys.argv[1]
    new_row_path = sys.argv[2]
    row_idx      = int(sys.argv[3])
    out_path     = sys.argv[4] if len(sys.argv) > 4 else sheet_path

    sheet   = Image.open(sheet_path).convert("RGBA")
    new_row = Image.open(new_row_path).convert("RGBA")

    SW, SH = sheet.size
    RW, RH = new_row.size

    cell_h = SH // N_ROWS          # height of one row in the sheet
    y_top  = row_idx * cell_h       # top pixel of the target row

    state_name = STATE_NAMES[row_idx] if row_idx < len(STATE_NAMES) else str(row_idx)
    print(f"Sheet:    {SW}×{SH}  (cell_h={cell_h}px)")
    print(f"New row:  {RW}×{RH}")
    print(f"Replacing row {row_idx} ({state_name}) at y={y_top}..{y_top+cell_h}")

    # Scale new_row to match sheet width × cell_h if needed
    if (RW, RH) != (SW, cell_h):
        print(f"  Resizing new row {RW}×{RH} → {SW}×{cell_h}")
        new_row = new_row.resize((SW, cell_h), Image.LANCZOS)

    # Blank out the target row, then paste the replacement
    result = sheet.copy()
    blank  = Image.new("RGBA", (SW, cell_h), (255, 255, 255, 255))
    result.paste(blank,   (0, y_top))
    result.paste(new_row, (0, y_top), new_row.split()[3])

    result.save(out_path)
    print(f"Saved → {out_path}")

if __name__ == "__main__":
    main()
