## How to use it

1) Create a PR
2) Tag it with "Needs Documentation"
3) Optionally set the milestone to it
4) Merge it

Then
* an issue is automatically created in the QGIS-Documentation repository
* in the PR, a message pings the author to tell him he should take care of the documentation issue

Example:
* Dummy PR: https://github.com/qgis/QGIS/pull/33627 
* Auto created issue: https://github.com/qgis/QGIS-Documentation/issues/4744

## How it works

* A Github [workflow]( https://github.com/qgis/QGIS/blob/master/.github/workflows/pr_to_doc_issue.yml) takes care of creating the issue and commenting.
* To make it works on jobs triggered from forks (almost all PRs actually), the Github token has to be manually given. Since it cannot be written in clear, an [action](https://github.com/opengisch/clear-token) obfuscates its using xor encryption.
* The labels on QGIS-Documentation are deduced from the PR milestone (e.g. 3.10.2 => 3.10). To label issues, push access is required. Due to security reasons (the token appears in clear), qgis-bot has no specific rights. Labels are created using a [workflow](https://github.com/qgis/QGIS-Documentation/blob/master/.github/workflows/auto-label.yml).


