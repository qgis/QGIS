import os
import shutil
import subprocess
import sys

import requests

TILE_SIDE = 3
AVATARS_PER_TILE = TILE_SIDE * TILE_SIDE
MOSAIC_WIDTH_TILES = 6
MOSAIC_HEIGHT_TILES = 3
NUMBER_OF_TILES = MOSAIC_WIDTH_TILES * MOSAIC_HEIGHT_TILES - 2
NUMBER_OF_AVATARS = NUMBER_OF_TILES * AVATARS_PER_TILE

FONT = "Source-Sans-Pro-Bold"
GREY = "#303030"
GREEN = "#5d9933"
TILE_SIZE = 400

ALTERNATE_AVATARS_DIR = "alternate-avatars"
QGIS_ICON_PATH = "../../icons/qgis_icon.svg"


def check_font_exists(font_name):
    try:
        result = subprocess.run(
            ["convert", "-list", "font"], capture_output=True, text=True, check=True
        )
        if font_name not in result.stdout:
            print(f"Error: Font {font_name} not found.")
            sys.exit(1)
    except (CalledProcessError, FileNotFoundError) as e:
        print(f"Error checking fonts (Is ImageMagick installed?): {e}")
        sys.exit(1)


def download_avatars():
    for t in range(NUMBER_OF_TILES):
        tile_dir = f"avatars_{t:02d}"
        shutil.rmtree(tile_dir, ignore_errors=True)
        os.makedirs(tile_dir, exist_ok=True)

    api_url = "https://raw.githubusercontent.com/qgis/QGIS-Website/refs/heads/main/data/contributors/contributors.json"
    headers = {"Accept": "application/vnd.github.v3+json"}

    print(f"fetching {api_url}")
    try:
        response = requests.get(api_url, headers=headers)
        response.raise_for_status()
        contributors_data = response.json()
    except requests.exceptions.RequestException as e:
        print(f"Error downloading contributors JSON: {e}")
        sys.exit(1)

    i = 0
    for contributor in contributors_data["contributors"]:
        if i >= NUMBER_OF_AVATARS:
            break

        login = contributor.get("login", "unknown")
        url = contributor.get("avatar_url", "")
        if not login or not url:
            print(f"...Skipping {login}, no avatar URL")
            continue

        print(f"Downloading avatar {i + 1}/{NUMBER_OF_AVATARS} for {login} from {url}")

        t = i // AVATARS_PER_TILE
        tile_dir = f"avatars_{t:02d}"
        output = os.path.join(tile_dir, f"{i:04d}.png")

        alternate_path = os.path.join(ALTERNATE_AVATARS_DIR, f"{login}.png")

        if os.path.exists(alternate_path):
            shutil.copyfile(alternate_path, output)
        else:
            try:
                avatar_response = requests.get(url, stream=True)
                avatar_response.raise_for_status()
                with open(output, "wb") as avatar_file:
                    shutil.copyfileobj(avatar_response.raw, avatar_file)
                i += 1
            except requests.exceptions.RequestException as e:
                print(f"Error downloading avatar for {login}: {e}")
                continue


def create_tiles():
    mosaic_dir = "mosaic"
    shutil.rmtree(mosaic_dir, ignore_errors=True)
    os.makedirs(mosaic_dir, exist_ok=True)

    for t in range(NUMBER_OF_TILES):
        print(f"Generating tile {t + 1}/{NUMBER_OF_TILES}")
        tile_dir = f"avatars_{t:02d}"
        output_file = os.path.join(mosaic_dir, f"mosaic{t:02d}0.png")
        input_files = os.path.join(tile_dir, "*.png")

        command = [
            "montage",
            "-background",
            GREY,
            "-geometry",
            f"200x200+{TILE_SIDE - 1}+{TILE_SIDE - 1}",
            "-tile",
            f"{TILE_SIDE}x{TILE_SIDE}",
            input_files,
            output_file,
        ]
        subprocess.run(command)

    qgis_block_file = os.path.join(mosaic_dir, "mosaic071.png")
    command = [
        "convert",
        "-background",
        GREY,
        "-density",
        "1200",
        "-resize",
        "320x320",
        "-gravity",
        "Center",
        "-extent",
        f"{TILE_SIZE}x{TILE_SIZE}",
        QGIS_ICON_PATH,
        qgis_block_file,
    ]
    subprocess.run(command)

    dev_block_file = os.path.join(mosaic_dir, "mosaic072.png")
    command = [
        "convert",
        "-size",
        f"{TILE_SIZE}x{TILE_SIZE}",
        "xc:" + GREEN,
        "-gravity",
        "Center",
        "-font",
        FONT,
        "-fill",
        GREY,
        "-pointsize",
        "200",
        "-annotate",
        "0",
        "dev",
        dev_block_file,
    ]
    subprocess.run(command)


def compute_and_resize_mosaic():
    mosaic_file = "mosaic.png"
    input_files = "mosaic/*.png"

    command = [
        "montage",
        "-background",
        GREY,
        "-geometry",
        f"{TILE_SIZE}x{TILE_SIZE}",
        "-tile",
        f"{MOSAIC_WIDTH_TILES}x{MOSAIC_HEIGHT_TILES}",
        input_files,
        mosaic_file,
    ]
    subprocess.run(command)

    mosaic_width = TILE_SIZE * MOSAIC_WIDTH_TILES
    mosaic_height = TILE_SIZE * MOSAIC_HEIGHT_TILES + TILE_SIZE // TILE_SIDE

    final_output_path = "../splash.png"

    command = [
        "convert",
        mosaic_file,
        "-resize",
        f"{mosaic_width}x{mosaic_height}",
        "-background",
        GREY,
        "-gravity",
        "North",
        "-extent",
        f"{mosaic_width}x{mosaic_height}",
        final_output_path,
    ]
    subprocess.run(command)


def main():
    keep_avatars = False
    if len(sys.argv) > 1 and sys.argv[1] == "--keep-avatars":
        keep_avatars = True

    check_font_exists(FONT)

    if not keep_avatars:
        download_avatars()

    create_tiles()

    compute_and_resize_mosaic()


if __name__ == "__main__":
    main()
