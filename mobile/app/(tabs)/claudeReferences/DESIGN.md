---
name: Obsidian Grove
colors:
  surface: '#121414'
  surface-dim: '#121414'
  surface-bright: '#38393a'
  surface-container-lowest: '#0d0e0f'
  surface-container-low: '#1a1c1c'
  surface-container: '#1e2020'
  surface-container-high: '#282a2b'
  surface-container-highest: '#333535'
  on-surface: '#e2e2e2'
  on-surface-variant: '#c3c8c0'
  inverse-surface: '#e2e2e2'
  inverse-on-surface: '#2f3131'
  outline: '#8d928b'
  outline-variant: '#434843'
  surface-tint: '#b8ccb8'
  primary: '#b8ccb8'
  on-primary: '#243427'
  primary-container: '#1b2b1e'
  on-primary-container: '#819382'
  inverse-primary: '#516353'
  secondary: '#b2ceb4'
  on-secondary: '#1e3524'
  secondary-container: '#364f3b'
  on-secondary-container: '#a4bfa6'
  tertiary: '#c1c9bf'
  on-tertiary: '#2b322b'
  tertiary-container: '#222922'
  on-tertiary-container: '#899087'
  error: '#ffb4ab'
  on-error: '#690005'
  error-container: '#93000a'
  on-error-container: '#ffdad6'
  primary-fixed: '#d4e8d4'
  primary-fixed-dim: '#b8ccb8'
  on-primary-fixed: '#0f1f13'
  on-primary-fixed-variant: '#3a4b3c'
  secondary-fixed: '#cdeacf'
  secondary-fixed-dim: '#b2ceb4'
  on-secondary-fixed: '#082010'
  on-secondary-fixed-variant: '#344c39'
  tertiary-fixed: '#dde5da'
  tertiary-fixed-dim: '#c1c9bf'
  on-tertiary-fixed: '#161d17'
  on-tertiary-fixed-variant: '#414941'
  background: '#121414'
  on-background: '#e2e2e2'
  surface-variant: '#333535'
typography:
  display-circle:
    fontFamily: Montserrat
    fontSize: 48px
    fontWeight: '300'
    lineHeight: 52px
    letterSpacing: -0.02em
  headline-lg:
    fontFamily: Montserrat
    fontSize: 32px
    fontWeight: '500'
    lineHeight: 40px
    letterSpacing: -0.01em
  headline-lg-mobile:
    fontFamily: Montserrat
    fontSize: 24px
    fontWeight: '500'
    lineHeight: 32px
  body-md:
    fontFamily: Montserrat
    fontSize: 16px
    fontWeight: '400'
    lineHeight: 24px
    letterSpacing: 0.01em
  label-sm:
    fontFamily: Montserrat
    fontSize: 12px
    fontWeight: '600'
    lineHeight: 16px
    letterSpacing: 0.05em
rounded:
  sm: 0.25rem
  DEFAULT: 0.5rem
  md: 0.75rem
  lg: 1rem
  xl: 1.5rem
  full: 9999px
spacing:
  unit: 8px
  container-padding: 24px
  gutter: 16px
  stack-gap: 12px
  section-margin: 40px
---

## Brand & Style

The design system is centered on a premium, minimalist ethos tailored for a circular hardware ecosystem. It evokes a sense of "digital sanctuary"—a calm, technical interface that feels more like a precision instrument than a typical mobile app. The target audience values high-end industrial design, quiet luxury, and focused utility.

The aesthetic blends **Minimalism** with **Glassmorphism** and **Tactile** influences. It utilizes heavy whitespace (negative space) to draw focus to the circular center of the interaction model. Surfaces are treated as physical layers of obsidian and dark glass, creating a sophisticated environment that mirrors the circular physical device. The emotional response is one of clarity, silence, and high-fidelity control.

## Colors

The palette is deeply monochromatic and desaturated, leaning into the shadows of a forest at night. 

- **Primary Background**: A deep, soft black (#0A0A0A) provides the canvas, ensuring the hardware bezel of the phone disappears into the interface.
- **Accent Forest**: The desaturated forest green (#1B2B1E) is used for primary containers and subtle depth.
- **Interactive Layers**: Lighter green variants are reserved for interactive states and thin, low-contrast borders.
- **Typography**: Muted off-white (#E0E0E0) ensures high legibility without the harshness of pure white, maintaining the low-light comfort of the experience.
- **Gradients**: Use extremely subtle radial gradients (Primary to Tertiary) to mimic the way light hits a curved lens or a circular display.

## Typography

This design system utilizes a clean, modern sans-serif to maintain a technical and architectural feel. 

- **Weight Strategy**: Use "Light" (300) for large numerical displays to emphasize precision, and "Medium" (500) for functional headings.
- **Scale**: Typography remains spacious. Tracking (letter-spacing) is slightly increased on small labels to ensure legibility against dark backgrounds, while display type uses tighter tracking for a premium "editorial" feel.
- **Alignment**: Center-alignment is preferred for hero sections to mirror the circular nature of the companion device, while list-based data defaults to left-aligned for rapid scanning.

## Layout & Spacing

The layout philosophy follows a **Fluid Grid** with generous safe areas. 

- **Circular Context**: Primary interactions and status indicators are housed in a central "orbital" layout, where elements are positioned relative to a central axis.
- **Rhythm**: An 8px base unit drives all spacing. 
- **Desktop/Tablet**: On larger screens, the interface does not stretch; instead, it maintains a narrow, focused central column (max 480px) to simulate the focused nature of a mobile/watch experience.
- **Margins**: Use aggressive 24px side margins to prevent content from feeling "crowded" by the edges of the physical screen.

## Elevation & Depth

Depth is conveyed through **Tonal Layers** and **Low-Contrast Outlines**.

- **Surfaces**: Elements do not use drop shadows. Instead, elevation is defined by a slight lightening of the background color. A level 1 container uses `#131313`, while a level 2 container uses `#1B2B1E`.
- **Borders**: All cards and buttons feature a 1px solid border. The color is a slightly lighter version of the container surface (e.g., `#2D4532`), creating a "etched glass" effect.
- **Backdrop**: For overlays and modals, use a heavy backdrop blur (20px) with a 60% opacity fill of the background color to maintain a sense of environmental continuity.

## Shapes

The shape language is defined by the circle. 

- **Card Radius**: All primary cards use a `rounded-lg` (16px) or `rounded-xl` (24px) corner radius to soften the technical aesthetic.
- **Interactive Elements**: Buttons and input fields use a fully rounded "Pill" shape to echo the circular device.
- **Iconography**: Icons should be enclosed in circular frames or use rounded terminals. Sharp corners are strictly avoided to maintain the "soft technical" feel.

## Components

- **Buttons**: Primary buttons are pill-shaped with a forest-green fill (#1B2B1E) and off-white text. Secondary buttons use a ghost style with a 1px border.
- **Cards**: Background-toned containers with 1px borders. Use cards to group related telemetry from the round device.
- **Circular Progress**: The "hero" component of the app. Thin 2px strokes for progress tracks, using subtle green gradients for "active" states.
- **Input Fields**: Minimalist underlines or pill-shaped containers with low-contrast borders. The focus state is indicated by a subtle glow of the border, not a change in fill.
- **Lists**: List items are separated by generous vertical spacing (12px) rather than divider lines, using "invisible" containers to maintain the spacious layout.
- **Chips**: Small, pill-shaped tags used for status (e.g., "Connected", "Syncing"). These should be monochromatic and never use loud, vibrating colors.