import json
import re

import click
import requests


@click.command()
@click.option(
    "--release",
    help="Which release to extract. `ltr` or `stable` assume release are tagged as final-x_x_x or ltr-x_x_x",
)
@click.option(
    "--github_token",
    default=None,
    help="Github token. Can help in case of rate limits.",
)
def extract(release, github_token):
    r = requests.get(
        "https://api.github.com/repos/qgis/QGIS/git/refs/tags",
        headers={"Authorization": github_token},
    )
    r.raise_for_status()
    tags = json.loads(r.text)
    releases = dict()
    current_ltr = None
    current_release = None
    for tag in tags:
        ref = tag["ref"]
        tag_name = ref.split("/")[-1]
        if tag_name.startswith("final-"):
            version_parts = re.split(r"[\-_]", tag_name)[1:]
            if int(version_parts[0]) >= 3:
                releases[version_parts[0] + "." + version_parts[1]] = ".".join(
                    version_parts
                )
                current_release = version_parts[0] + "." + version_parts[1]
        if tag_name.startswith("ltr-"):
            version_parts = re.split(r"[\-_]", tag_name)[1:]
            version = ".".join(version_parts)
            if version != current_release:
                current_ltr = version

    info = {
        "ltr": {
            "short_version": current_ltr,
            "patch_version": releases[current_ltr],
            "tag_name": f'final-{releases[current_ltr].replace(".", "_")}',
        },
        "stable": {
            "short_version": current_release,
            "patch_version": releases[current_release],
            "tag_name": f'final-{releases[current_release].replace(".", "_")}',
        },
    }

    print(f"QGIS_VERSION_STABLE_PATCH={info['stable']['patch_version']}")
    print(f"QGIS_VERSION_LTR_PATCH={info['ltr']['patch_version']}")

    print(f"QGIS_VERSION_SHORT={info[release]['short_version']}")
    print(f"QGIS_VERSION_PATCH={info[release]['patch_version']}")
    print(f"QGIS_VERSION_TAG={info[release]['tag_name']}")


if __name__ == "__main__":
    extract()
