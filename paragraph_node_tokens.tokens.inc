<?php

use Drupal\Core\Render\BubbleableMetadata;
use Drupal\image\Entity\ImageStyle;

/**
 * Implements hook_token_info().
 */
function paragraph_node_tokens_token_info() {
  $info = array();

  $info['tokens']['node']['paragraph_text'] = [
    'name' => t("Paragraph Text"),
    'description' => t("The full output of text-based paragraph fields"),
  ];
  
  return $info;
}

/**
 * Implements hook_tokens().
 */
function paragraph_node_tokens_tokens($type, $tokens, array $data, array $options, BubbleableMetadata $bubbleable_metadata) {

  
  $replacements = array();

  //
  // @TODO make this configurable by GUI
  //
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

      if ($paragraph_text_tokens = $token_service->findWithPrefix($tokens, 'paragraph_text')) {

        foreach( $paragraph_text_tokens as $token_key => $token_full ) {
          switch ($token_key) {
            case 'summary':
            case 'summary:single-line':
              $text_content = '';
    
              if ( $node = $data['node'] ) {
                
                foreach( $paragraph_fields as $paragraph_field ) {
    
                  if ( $node->hasField($paragraph_field) ) {
    
                    $paragraph_field_values = $node->get($paragraph_field)->getValue();

                    //
                    // Iterate over paragraph entities on this node
                    //
                    foreach( $paragraph_field_values as $paragraph_field_value ) {
                      if (empty($paragraph_field_value['target_id']) {
                         continue;
                      }

                      $paragraph_entity = \Drupal\paragraphs\Entity\Paragraph::load( $paragraph_field_value['target_id'] );
                      
                      //
                      // Check to see if this entity is one of our text-based paragraphs
                      //
                      if ( $paragraph_entity && in_array($paragraph_entity->getType(), $paragraph_text_types) ) {
                          
                          //
                          // Iterate over text fields, creating the text body
                          //
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
    
              $text_content = trim($text_content);
              $text_content = strip_tags($text_content);

              if ( !$text_content ) {
                //
                // Fallback to other field
                //
                if ( $fallback_text_fields ) {
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
              }

              $text_content = trim($text_content);
              $text_content = strip_tags($text_content);
              $text_content = html_entity_decode($text_content);
              $text_content = htmlspecialchars($text_content);
              $text_content = text_summary($text_content, $summary_formatter, $summary_length);
              
              if ( strpos($token_key, 'single-line') !== false ) {
                $text_content = str_replace("\n", ' ', $text_content);
              }

              $replacements[$original] = $text_content;
              break;
          }
        }
      }      
    }
  }
  // Return the replacements.
  return $replacements;
}
