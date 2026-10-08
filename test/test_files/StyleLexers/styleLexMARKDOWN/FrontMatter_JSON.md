;;;
{
  "title": "JSON Front Matter Example",
  "author": "Jane Doe",
  "published": true,
  "rating": 4.5
}
;;;

# JSON Front Matter

The `;;;` delimiter lines above MUST render as **Front Matter Marker**.
The `{` and `}` braces and keys (`title`, `author`, `published`,
`rating`) MUST render as **Front Matter Key**/**Front Matter**, and the
values after `:` MUST render as **Front Matter**.

Regular Markdown body text below is unaffected and keeps its normal
styling (e.g. **strong**, *emphasis*, `code`).
