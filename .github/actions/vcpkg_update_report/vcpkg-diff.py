import re


def extract_packages(data):
    """
    Extract package name, triplet, version, and features information from the file content.
    """
    packages = {}
    lines = data.strip().split("\n")
    for line in lines:
        # Regex to match the package format and capture features inside brackets
        match = re.match(
            r"\s*\*\s+([^\[\]:]+)(?:\[(.*?)\])?:([^\[\]@]+)@([^\s]+)\s+--", line
        )
        if match:
            package_name = match.group(1)
            features = match.group(2) if match.group(2) else ""
            triplet = match.group(3)
            version = match.group(4)
            features_list = (
                [feature.strip() for feature in features.split(",")] if features else []
            )
            packages[package_name] = (triplet, version, features_list)
    return packages


def compare_features(features1, features2):
    """
    Compare two feature lists and return the differences.
    """
    added_features = set(features2) - set(features1)
    removed_features = set(features1) - set(features2)
    return added_features, removed_features


def generate_report(file1_content, file2_content):
    # Extract package information from both files
    file1_packages = extract_packages(file1_content)
    file2_packages = extract_packages(file2_content)

    added = []
    removed = []
    updated = []

    # Identify removed and updated packages
    for pkg in file1_packages:
        if pkg not in file2_packages:
            removed.append(pkg)
        else:
            # Compare version and features
            triplet1, version1, features1 = file1_packages[pkg]
            triplet2, version2, features2 = file2_packages[pkg]
            updated_parts = []
            if version1 != version2 or triplet1 != triplet2:
                updated_parts.append(f"{version1} -> {version2}")
            added_features, removed_features = compare_features(features1, features2)
            if added_features:
                updated_parts.append("+" + ", ".join(added_features))
            if removed_features:
                updated_parts.append("-" + ", ".join(removed_features))
            if updated_parts:
                updated.append(f"{pkg}: " + " ".join(updated_parts))

    # Identify added packages
    for pkg in file2_packages:
        if pkg not in file1_packages:
            added.append(pkg)

    # Print the report
    if added:
        print("**Added packages:**")
    for pkg in added:
        triplet, version, features = file2_packages[pkg]
        print(f" 🍓 {pkg}: {version} (Features: {', '.join(features)})")

    if removed:
        print("\n**Removed packages:**")
    for pkg in removed:
        triplet, version, features = file1_packages[pkg]
        print(f" 🍄 {pkg}: {version} (Features: {', '.join(features)})")

    if updated:
        print("\n**Updated packages:**")
    for pkg in updated:
        print(f" 🍇 {pkg}")


def read_file(file_path):
    """
    Read the content of a file.
    """
    with open(file_path) as file:
        return file.read()


# Read files
file1_content = read_file("/tmp/vcpkg-base-output.txt")
file2_content = read_file("/tmp/vcpkg-head-output.txt")

# Generate the report
generate_report(file1_content, file2_content)
