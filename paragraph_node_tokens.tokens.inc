<?php

use Drupal\Core\Render\BubbleableMetadata;

/**
 * Implements hook_token_info().
 */
function paragraph_node_tokens_token_info() {
  $info = array();

  $info['types']['paragraph_text'] = [
    'name' => t("Paragraph Text"),
    'description' => t("The full output of text-based paragraph fields"),
    'nested' => TRUE,
  ];
  $info['tokens']['node']['paragraph_text'] = [
    'name' => t("Paragraph Text"),
    'description' => t("The full output of text-based paragraph fields, compiled into a large block"),
    'type' => 'paragraph_text',
  ];
  $info['tokens']['paragraph_text']['full-length'] = [
    'name' => t("Compiled"),
    'description' => t("The full output of text-based paragraph fields, compiled into a large block"),
  ];
  $info['tokens']['paragraph_text']['summary'] = [
    'name' => t("Summarized"),
    'description' => t("The full output of text-based paragraph fields, compiled into a large block and trimmed to standard teaser length"),
  ];
  $info['tokens']['paragraph_text']['one-line'] = [
    'name' => t("Single Line Summary"),
    'description' => t("The full output of text-based paragraph fields, compiled into a single line and trimmed to standard teaser length"),
  ];
  return $info;
}

/**
 * Implements hook_tokens().
 */
function paragraph_node_tokens_tokens($type, $tokens, array $data, array $options, BubbleableMetadata $bubbleable_metadata) {
  $replacements = array();

  // @TODO make this configurable by GUI
  $paragraph_fields = \Drupal::config('paragraph_node_tokens.settings')->get('paragraph_fields');
  $paragraph_text_types = \Drupal::config('paragraph_node_tokens.settings')->get('paragraph_text_types');
  $paragraph_text_fields = \Drupal::config('paragraph_node_tokens.settings')->get('paragraph_text_fields');
  $fallback_text_fields = \Drupal::config('paragraph_node_tokens.settings')->get('fallback_text_fields');
  
  if ($type == 'node') {

    $token_service = \Drupal::token();
    $settings = \Drupal::service('plugin.manager.field.formatter')->getDefaultSettings('text_summary_or_trimmed');
    $summary_length = $settings['trim_length'];
    $summary_formatter = 'plain_text';

    foreach ($tokens as $name => $original) {
      if ($name == 'paragraph_text') {
        $paragraph_text_tokens = ['full-length' => 'full-length'];
      }
      else {
        $paragraph_text_tokens = $token_service->findWithPrefix($tokens, 'paragraph_text');
      }

      if ($paragraph_text_tokens) {
        $token_key = array_key_first($paragraph_text_tokens);
        $text_content = '';
        if ( $node = $data['node'] ) {
          foreach( $paragraph_fields as $paragraph_field ) {
            if ( $node->hasField($paragraph_field) ) {
              $paragraph_field_values = $node->get($paragraph_field)->getValue();
              // Iterate over paragraph entities on this node
              foreach( $paragraph_field_values as $paragraph_field_value ) {
                $paragraph_entity = \Drupal\paragraphs\Entity\Paragraph::load( $paragraph_field_value['target_id'] );
                // Check to see if this entity is one of our text-based paragraphs
                if ( $paragraph_entity && in_array($paragraph_entity->getType(), $paragraph_text_types) ) {
                    // Iterate over text fields, creating the text body
                    foreach( $paragraph_text_fields as $text_field ) {
                        if ( $paragraph_entity->hasField($text_field) ) {
                          $text_values = $paragraph_entity->get($text_field)->getValue();

                          foreach( $text_values as $text_value ) {
                            $text_content .= $text_value['value'];
                          }
                        }
                    }
                }
              }
            }
          }
        }

        // Fallback to other field
        if ( !$text_content && $fallback_text_fields ) {
          foreach( $fallback_text_fields as $text_field ) {
            if ( $node->hasField($text_field) ) {
              $text_values = $node->get($text_field)->getValue();
              foreach( $text_values as $text_value ) {
                $text_content = trim($text_value['value']);
                if ( $text_content ) {
                  break 2; //We only use one fallback field. So if we found one, exit the loop.
                }
              }
            }
          }
        }

        $text_content = strip_tags($text_content);
        $text_content = trim($text_content);
        $text_content = html_entity_decode($text_content);
        $text_content = htmlspecialchars($text_content);

        // Summarize text.
        if ($token_key == 'summary' || $token_key == 'one-line') {
          $text_content = text_summary($text_content, $summary_formatter, $summary_length);
        }
        // Single-line text.
        if ($token_key == 'one-line') {
          $text_content = str_replace("\n", ' ', $text_content);
        }

        $replacements[$original] = $text_content;
      }
    }
  }
  // Return the replacements.
  return $replacements;
}