+++
title = "TOML Front Matter Example"
author = "Jane Doe"
published = true
rating = 4.5
+++

# TOML Front Matter

The `+++` delimiter lines above MUST render as **Front Matter Marker**.
Keys (`title`, `author`, `published`, `rating`) MUST render as
**Front Matter Key**, and the values after `=` MUST render as
**Front Matter**.

Regular Markdown body text below is unaffected and keeps its normal
styling (e.g. **strong**, *emphasis*, `code`).
