<?php

/**
 * @file facebook_tracking_pixel.admin.inc
 * Admin pages.
 */
use UnitedPrototype\GoogleAnalytics\Exception;

/**
 * Form builder.
 */
function facebook_tracking_pixel_settings_form($form, &$form_state) {
  $form_state['storage']['facebook_tracking_pixel_tracking_codes'] = isset($form_state['storage']['facebook_tracking_pixel_tracking_codes']) ? $form_state['storage']['facebook_tracking_pixel_tracking_codes'] : 0;
  $form['intro_0'] = array(
    '#markup' => '<p>' . t('Global Facebook Tracking Pixel Codes.') . '</p>',
  );
  $form['intro_1'] = array(
    '#markup' => '<p>' . t('Do not put code into this module from an untrusted source. These inputs are') . ' <strong>' . t('NOT') . '</strong> ' . t('santized!') . '</p>',
  );
  $form['facebook_tracking_pixel_global_agree'] = array(
    '#type' => 'checkbox',
    '#title' => t('I accept full responsibility for what this code may do to my site.'),
    '#default_value' => variable_get('facebook_tracking_pixel_global_agree', 0),
  );
  $form['aggregation'] = array(
    '#type' => 'fieldset',
    '#description' => t('Aggregation settings for the javascripts. Default setting is to allow aggregation. Uncheck to disable aggregation of the tracking javascript codes.')
  );
  $form['aggregation']['facebook_tracking_pixel_aggregation'] = array(
    '#type' => 'checkbox',
    '#title' => 'JS Aggregation',
    '#default_value' => variable_get('facebook_tracking_pixel_aggregation', TRUE),
  );

  $form['facebook_tracking_pixel_path'] = array(
    '#title' => t('File storage path'),
    '#type' => 'textfield',
    '#default_value' => variable_get('facebook_tracking_pixel_path', 'public://facebook_tracking_pixel_path'),
    '#description' => t('The subdirectory of the system files directory where Facebook Tracking Pixel will store its files. Note: changing this path will cause the old path to move to the new path, overwriting the new path, if it exists.'),
  );

  $existing_codes = variable_get('facebook_tracking_pixel_tracking_codes', NULL);
  if (!empty($existing_codes['code']) && is_array($existing_codes['code'])) {
    // Number of existing codes.
    if (empty($form_state['num_names'])) {
      $form_state['num_names'] = count($existing_codes['code']);
    }
    foreach ($existing_codes['code'] as $key => $value) {
      $existing_codes_value[$key] = $value;
    }
  }


  $form['facebook_tracking_pixel_tracking_codes'] = array(
    '#type' => 'fieldset',
    '#title' => t('Facebook Tracking Codes'),
    '#description' => t('Paste the tracking codes as they have been provided to you by Facebook either via email or by clicking "view code" gear icon in the pixel management page.'),
    // Set up the wrapper so that AJAX will be able to replace the fieldset.
    '#prefix' => '<div id="tracking-codes-wrapper">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
  );

  // Build the fieldset with the proper number of names. We'll use
  // $form_state['num_names'] to determine the number of textfields to build.
  if (empty($form_state['num_names'])) {
    $form_state['num_names'] = 1;
  }
  for ($i = 0; $i < $form_state['num_names']; $i++) {
    $form['facebook_tracking_pixel_tracking_codes']['code'][$i] = array(
      '#title' => t('Code'),
      '#type' => 'textarea',
    );
    $form['facebook_tracking_pixel_tracking_codes']['code'][$i]['#default_value'] = isset($existing_codes_value[$i]) ? $existing_codes_value[$i] : '';
  }
  $form['facebook_tracking_pixel_tracking_codes']['add_name'] = array(
    '#type' => 'submit',
    '#value' => t('Add one more'),
    '#submit' => array('facebook_tracking_pixel_add_more_add_one'),
    '#ajax' => array(
      'callback' => 'facebook_tracking_pixel_add_more_callback',
      'wrapper' => 'tracking-codes-wrapper',
    ),
  );
  if ($form_state['num_names'] > 1) {
    $form['facebook_tracking_pixel_tracking_codes']['remove_name'] = array(
      '#type' => 'submit',
      '#value' => t('Remove one'),
      '#submit' => array('facebook_tracking_pixel_add_more_remove_one'),
      '#ajax' => array(
        'callback' => 'facebook_tracking_pixel_add_more_callback',
        'wrapper' => 'tracking-codes-wrapper',
      ),
    );
  }
  $form['facebook_tracking_pixel_tracking_codes']['add_name'] = array(
    '#type' => 'submit',
    '#value' => t('Add one more'),
    '#submit' => array('facebook_tracking_pixel_add_more_add_one'),
    '#ajax' => array(
      'callback' => 'facebook_tracking_pixel_add_more_callback',
      'wrapper' => 'tracking-codes-wrapper',
    ),
  );
  if ($form_state['num_names'] > 1) {
    $form['facebook_tracking_pixel_tracking_codes']['remove_name'] = array(
      '#type' => 'submit',
      '#value' => t('Remove one'),
      '#submit' => array('facebook_tracking_pixel_add_more_remove_one'),
      '#ajax' => array(
        'callback' => 'facebook_tracking_pixel_add_more_callback',
        'wrapper' => 'tracking-codes-wrapper',
      ),
    );
  }
  // Add a custom submission handler.
  $form['#submit'][] = 'facebook_tracking_pixel_settings_submit';
  return system_settings_form($form);
}

/**
 * Callback for both ajax-enabled buttons.
 *
 * Selects and returns the fieldset with the names in it.
 */
function facebook_tracking_pixel_add_more_callback($form, $form_state) {
  return $form['facebook_tracking_pixel_tracking_codes'];
}

/**
 * Submit handler for the "add-one-more" button.
 *
 * Increments the max counter and causes a rebuild.
 */
function facebook_tracking_pixel_add_more_add_one($form, &$form_state) {
  $form_state['num_names']++;
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for the "remove one" button.
 *
 * Decrements the max counter and causes a form rebuild.
 */
function facebook_tracking_pixel_add_more_remove_one($form, &$form_state) {
  if ($form_state['num_names'] > 1) {
    $form_state['num_names']--;
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Implements hook_submit().
 * Settings form - submit callback.
 */
function facebook_tracking_pixel_settings_submit($form, &$form_state) {
  // Trim slashes from the path.
  $form_state['values']['facebook_tracking_pixel_path'] = trim($form_state['values']['facebook_tracking_pixel_path'], '/');

  // If the path changed, notify that the folder must be moved.
  $old_path = variable_get('facebook_tracking_pixel_path', 'public://facebook_tracking_pixel_path');
  if ($form_state['values']['facebook_tracking_pixel_path'] != $old_path and file_exists(drupal_realpath($old_path))) {
    $base_path = variable_get('file_' . file_default_scheme() . '_path', conf_path() . '/files') . '/';
    drupal_set_message(t('The file storage path has changed; thus, the contents of %old_path must manually be moved to %new_path.', array(
      '%old_path' => $base_path . file_uri_target($old_path),
      '%new_path' => $base_path . file_uri_target($form_state['values']['facebook_tracking_pixel_path']),
    )), 'warning', FALSE);
  }

  // The data from Facebook has to be parsed to extract the script and the
  // noscript information. We use PHP DOM processing to make it easy.


  if (!empty($form_state['values']['facebook_tracking_pixel_tracking_codes']['code']) && is_array($form_state['values']['facebook_tracking_pixel_tracking_codes']['code'])) {
    $fb_script = array();
    $fb_noscript = array();
    foreach ($form_state['values']['facebook_tracking_pixel_tracking_codes']['code'] as $script) {
      $script = filter_xss($script);
      $domd = new DOMDocument();
      libxml_use_internal_errors(TRUE);
      $domd->loadHTML($script);
      libxml_use_internal_errors(FALSE);
      $items = $domd->getElementsByTagName('script');
      foreach ($items as $item) {
        $fb_script[] = $domd->saveHTML($item->firstChild);
      }
      $items = $domd->getElementsByTagName('noscript');
      foreach ($items as $item) {
        $fb_noscript[] = $domd->saveHTML($item->firstChild);
      }
    }
  }
  // Remove all existing files, if present.
  try {
    facebook_tracking_pixel_delete_file();
  }
  catch (Exception $e) {
    watchdog('facebook_tracking_pixel', 'Failed deleting global JS files for Facebook Tracking Pixel. Error @error', array('@error' => $e), WATCHDOG_ERROR, NULL);
    return form_set_error('facebook_tracking_pixel_tracking_codes', 'Could not delete existing js files', NULL);
  };
  // Save the global js files for tracking pixel.
  $i = 0;
  foreach ($fb_script as $item) {
    facebook_tracking_pixel_save_file($item, 'fb_tkpx.' . $i . '.js');
    $i++;
  }
  variable_set('facebook_tracking_pixel_noscripts', $fb_noscript);

}
