from PIL import Image
import os

# --- Configuration ---
image_filename = "Upscaled Whole Sheets-3-4.png" # Your input PNG file
output_filename = "my_sprite.h"            # Output C header file
variable_name = "sprite_data"              # C array variable name
define_prefix = "SPRITE"                   # Prefix for WIDTH/HEIGHT defines

# Define Magenta as the color key (in both RGB888 and RGB565 formats)
key_color_rgb = (255, 0, 255) # Magenta (R=255, G=0, B=255)
key_color_565 = ((key_color_rgb[0] >> 3) << 11) | ((key_color_rgb[1] >> 2) << 5) | (key_color_rgb[2] >> 3) # Should be 0xF81F

# Alpha threshold: Pixels with alpha below this value will be replaced by the key color
alpha_threshold = 128
# ---------------------

try:
    script_dir = os.path.dirname(__file__)
    image_path = os.path.join(script_dir, image_filename)
    output_path = os.path.join(script_dir, output_filename)

    print(f"Opening image: {image_path}")
    img = Image.open(image_path)
    if img.mode != 'RGBA':
        img = img.convert('RGBA') # Ensure we have Alpha channel

    width, height = img.size
    pixels_rgba = list(img.getdata())

    print(f"Image size: {width}x{height}")
    print(f"Using Magenta (0x{key_color_565:04X}) as transparent color key.")
    print(f"Writing C header (NO byte swap) to: {output_path}") # Indicate NO swap

    with open(output_path, "w") as f:
        f.write("#pragma once\n\n")
        f.write("#include <cstdint>\n\n")
        f.write(f"#define {define_prefix}_WIDTH {width}\n")
        f.write(f"#define {define_prefix}_HEIGHT {height}\n\n")
        f.write(f"// RGB565 format (16 bits per pixel)\n")
        f.write(f"// Magenta (0x{key_color_565:04X}) is used as transparent color key\n")
        f.write(f"const uint16_t {variable_name}[{width * height}] = {{\n  ")

        count = 0
        for i, p in enumerate(pixels_rgba):
            r, g, b, a = p

            if a < alpha_threshold:
                rgb565 = key_color_565 # Use the key color
            else:
                # Convert the pixel's RGB to RGB565
                rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)

            # Write the NORMAL (non-swapped) value as hex
            f.write(f"0x{rgb565:04X}")
            count += 1

            if i < len(pixels_rgba) - 1:
                f.write(",")
            if count % 12 == 0:
                f.write("\n  ")
            elif i < len(pixels_rgba) - 1:
                 f.write(" ")

        f.write("\n}; // End of array\n")

    print(f"Successfully generated {output_filename}")

except FileNotFoundError:
    print(f"ERROR: Image file not found at '{image_path}'")
    print("Make sure the image file is in the same directory as the script, or update the 'image_filename' variable.")
except Exception as e:
    print(f"An error occurred: {e}")