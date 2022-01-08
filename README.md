# paragraph_node_tokens
a Drupal 8 module that provides a token to pull text content from a paragraph type

creates the following tokens:

```
[node:paragraph_text:summary]
[node:paragraph_text:summary:single-line]
```

You probably want to use the second one, which trims newlines.

## Before installing the module

Edit the config/install/paragraph_node_tokens.settings with the appropriate machine names:

```
paragraph_fields:
  - field_basic_page_body_content
  - field_news_item_body_content
  - field_story_body_content
paragraph_text_types:
  - text_block
paragraph_text_fields:
  - field_text_block_text_block
fallback_text_fields:
  - body
```

"paragraph_fields" are the paragraph reference fields attached to your nodes

"paragraph_text_types" is the name of the paragraph type that holds a text field

"paragraph_text_fields" Are the fields, attached to the paragraph entity, that are the actual text fields you want to pull token data

"fallback_text_fields" are fields to look at if no text is found in the paragraph field

## To update the module config

Update config/install/paragraph_node_tokens.settings and run a drush config import, which will look similar this this: `drush cim -y --partial --source=modules/contrib/paragraph_node_tokens/config/install/`