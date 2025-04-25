# Python Script: batch_convert_sprites.py (WITH EXTRA DEBUG PRINT)
import os
import glob # Used to find files matching a pattern
from PIL import Image
import re # Used for cleaning up names

# --- Configuration ---
# Folder containing your input PNG files (relative to this script's location)
input_folder = "sprites_png_input"

# Folder where the output .h files will be saved
# Make sure this path correctly points to your project's assets folder
output_folder = "../Digivice/assets" # From user's previous update

# Magenta Color Key (Pixels with low alpha become this)
key_color_rgb = (255, 0, 255) # Magenta (R=255, G=0, B=255)
alpha_threshold = 128       # Alpha values below this become the key color
# ---------------------

# --- Helper function to clean filenames for C variable/define names ---
def sanitize_name(filename):
    # Remove extension
    name = os.path.splitext(filename)[0]
    # Replace invalid characters (like hyphens, spaces) with underscores
    name = re.sub(r'[^a-zA-Z0-9_]', '_', name)
    # Ensure it doesn't start with a number (prepend underscore if it does)
    if name[0].isdigit():
        name = "_" + name
    return name
# --------------------------------------------------------------------

try:
    script_dir = os.path.dirname(os.path.abspath(__file__)) # Get script's directory reliably
    input_dir_path = os.path.join(script_dir, input_folder)
    output_dir_path = os.path.join(script_dir, output_folder)

    # Ensure output folder exists, create if not
    os.makedirs(output_dir_path, exist_ok=True)
    print(f"Output folder: {os.path.abspath(output_dir_path)}")

    # Find all .png files in the input folder
    search_pattern = os.path.join(input_dir_path, "*.png")
    png_files = glob.glob(search_pattern)

    if not png_files:
        print(f"Error: No PNG files found in '{input_dir_path}'")
        print("Please create the folder and place your PNG frames inside.")
    else:
        print(f"Found {len(png_files)} PNG files in '{input_dir_path}'. Starting conversion...")

    # Pre-calculate the key color in RGB565 format
    key_color_565 = ((key_color_rgb[0] >> 3) << 11) | ((key_color_rgb[1] >> 2) << 5) | (key_color_rgb[2] >> 3) # Should be 0xF81F

    # Loop through each found PNG file
    for image_path in png_files:
        base_filename = os.path.basename(image_path)
        sanitized_base = sanitize_name(base_filename) # Use helper to clean name

        # Define output names based on input filename
        output_h_filename = f"{sanitized_base}.h"
        variable_name = f"{sanitized_base}_data"
        define_prefix = sanitized_base.upper()
        output_path = os.path.join(output_dir_path, output_h_filename)

        print(f"\nProcessing '{base_filename}' ==>")
        print(f"  Output H file: '{output_h_filename}'")
        print(f"  Variable Name: '{variable_name}'")
        print(f"  Define Prefix: '{define_prefix}'")

        try:
            # --- Image processing ---
            img = Image.open(image_path)
            if img.mode != 'RGBA':
                img = img.convert('RGBA')
            width, height = img.size
            pixels_rgba = list(img.getdata())
            print(f"  Image size: {width}x{height}")

            # --- Calculate index for pixel (0, 9) for THIS image ---
            # Check if Y=9 is within the image height bounds first
            y_coord_to_check = 9
            if y_coord_to_check < height:
                 index_to_check = (y_coord_to_check * width) + 0 # X=0
                 print(f"  Checking pixel data at index: {index_to_check} (for X=0, Y={y_coord_to_check})")
            else:
                 index_to_check = -1 # Invalid index, won't match
                 print(f"  Y={y_coord_to_check} is out of bounds for image height {height}. Skipping pixel check.")


            # --- File writing ---
            with open(output_path, "w") as f:
                f.write("#pragma once\n\n")
                f.write("#include <cstdint>\n\n")
                f.write(f"#define {define_prefix}_WIDTH {width}\n")
                f.write(f"#define {define_prefix}_HEIGHT {height}\n\n")
                f.write(f"// RGB565 format (16 bits per pixel), Converted from {base_filename}\n")
                f.write(f"// Magenta (0x{key_color_565:04X}) is used as transparent color key\n")
                f.write(f"const uint16_t {variable_name}[{width * height}] = {{\n  ")

                count = 0
                for i, p in enumerate(pixels_rgba):
                    r, g, b, a = p

                    # --- Existing Debug Print for RGBA (only if index is valid) ---
                    if i == index_to_check:
                        print(f"  DEBUG: Pixel at index {i} (X=0, Y={y_coord_to_check}) - RGBA: ({r}, {g}, {b}, {a})")
                    # --- End Existing Debug ---

                    # Calculate rgb565 value
                    if a < alpha_threshold:
                        rgb565 = key_color_565
                    else:
                        rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)

                    # --- ADDED DEBUG PRINT FOR CALCULATED VALUE ---
                    if i == index_to_check: # <<< ADDED DEBUG LINE
                        print(f"  DEBUG: Calculated rgb565 value: 0x{rgb565:04X}") # <<< ADDED DEBUG LINE
                    # --- END ADDED DEBUG LINE ---

                    # Write the value that was just calculated
                    f.write(f"0x{rgb565:04X}")
                    if i < len(pixels_rgba) - 1:
                        f.write(",")
                    count += 1
                    if count % 12 == 0: # Formatting: Newline every 12 pixels
                        f.write("\n  ")
                    elif i < len(pixels_rgba) - 1:
                           f.write(" ") # Space after comma otherwise

                f.write("\n}; // End of array\n")
            print(f"  Successfully generated '{output_h_filename}'")

        except Exception as e:
            print(f"  ERROR processing '{base_filename}': {e}")

    print("\nBatch conversion finished.")

except Exception as e:
    print(f"A critical error occurred: {e}")