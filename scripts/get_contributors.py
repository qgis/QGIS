#!/usr/bin/env python3
###########################################################################
#    get_contributors.py
#    ---------------------
#    Date                 : October 2023
#    Copyright            : (C) 2023 by Loïc Bartoletti
#    Email                : loic dot bartoletti at oslandia dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

import requests

owner = "qgis"
repo = "qgis"

# Replace 'YOUR_TOKEN' with qgis/github Personal Access Token
token = 'YOUR_TOKEN'

base_url = f"https://api.github.com/repos/{owner}/{repo}/contributors"
page = 1

while True:
    # Build the URL with the page number and the number of items per page
    url = f"{base_url}?per_page=100&page={page}"

    headers = {"Authorization": f"token {token}"}

    response = requests.get(url, headers=headers)

    if response.status_code == 200:
        contributors = response.json()

        # If the list of contributors is empty, exit the loop
        if not contributors:
            break

        # Iterate through the contributors on this page
        for contributor in contributors:
            contributor_url = contributor["url"]
            contributor_data = requests.get(contributor_url).json()
            login = contributor_data["login"]
            name = contributor_data["name"]

            if name is None:
                name = login

            print(f"{name} ({login})")

        page += 1
    else:
        print(f"Error while fetching data (response code: {response.status_code})")
        break

