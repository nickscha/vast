import argparse
from pathlib import Path
from PIL import Image


def main() -> None:
    parser = argparse.ArgumentParser(description="Convert a PNG image to raw RGBA8 bytes.")
    parser.add_argument("input_png", type=Path, help="Path to source .png file")
    parser.add_argument("output_rgba", type=Path, help="Path to output .rgba raw file")
    parser.add_argument(
        "--flip-y",
        action="store_true",
        help="Flip image vertically (useful if OpenGL V=0 is bottom)",
    )
    args = parser.parse_args()

    img = Image.open(args.input_png).convert("RGBA")
    if args.flip_y:
        img = img.transpose(Image.Transpose.FLIP_TOP_BOTTOM)

    raw_bytes = img.tobytes("raw", "RGBA")
    args.output_rgba.parent.mkdir(parents=True, exist_ok=True)
    args.output_rgba.write_bytes(raw_bytes)

    width, height = img.size
    print(f"Wrote {len(raw_bytes)} bytes ({width}x{height} RGBA8) -> {args.output_rgba}")


if __name__ == "__main__":
    main()