<?php

/**
 * @file facebook_tracking_pixel.admin.inc
 * Admin pages.
 */


/**
 * Form builder for global tracking.
 */
function facebook_tracking_pixel_settings_form($form, &$form_state) {
  $form_state['storage']['facebook_tracking_pixel_tracking_codes'] = isset($form_state['storage']['facebook_tracking_pixel_tracking_codes']) ? $form_state['storage']['facebook_tracking_pixel_tracking_codes'] : 0;
  $form['intro_0'] = [
    '#markup' => '<p>' . t('Global Facebook Tracking Pixel Codes.') . '</p>',
  ];

  $form['intro_1'] = [
    '#markup' => '<p>' . t('Do not put code into this module from an untrusted source. These inputs are') . ' <strong>' . t('NOT') . '</strong> ' . t('santized!') . '</p>',
  ];

  $form['facebook_tracking_pixel_global_agree'] = [
    '#type' => 'checkbox',
    '#title' => t('I accept full responsibility for what this code may do to my site.'),
    '#default_value' => variable_get('facebook_tracking_pixel_global_agree', 0),
  ];

  $form['aggregation'] = [
    '#type' => 'fieldset',
    '#description' => t('Aggregation settings for the javascripts. Default setting is to allow aggregation. Uncheck to disable aggregation of the tracking javascript codes.')
  ];

  $form['aggregation']['facebook_tracking_pixel_aggregation'] = [
    '#type' => 'checkbox',
    '#title' => 'JS Aggregation',
    '#default_value' => variable_get('facebook_tracking_pixel_aggregation', TRUE),
  ];

  $form['facebook_tracking_pixel_path'] = [
    '#title' => t('File storage path'),
    '#type' => 'textfield',
    '#default_value' => variable_get('facebook_tracking_pixel_path', 'public://facebook_tracking_pixel_path'),
    '#description' => t('The subdirectory of the system files directory where Facebook Tracking Pixel will store its files. Note: changing this path will cause the old path to move to the new path, overwriting the new path, if it exists.'),
  ];

  $form['facebook_tracking_pixel_tracking_code'] = [
    '#title' => t('Code'),
    '#description' => t('Paste the tracking codes as they have been provided to you by Facebook either via email or by clicking "view code" under the actions menu.'),
    '#type' => 'textarea',
    '#default_value' => variable_get('facebook_tracking_pixel_tracking_code', TRUE),
  ];


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

  if (!empty($form_state['values']['facebook_tracking_pixel_tracking_code'])) {
    // The data from Facebook has to be parsed to extract the script and the
    // noscript information. We use PHP DOM processing to make it easy.
    $script = $form_state['values']['facebook_tracking_pixel_tracking_code'];
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

    // Remove existing file, if present.
    try {
      facebook_tracking_pixel_delete_file();
    }
    catch (Exception $e) {
      watchdog('facebook_tracking_pixel', 'Failed deleting global JS files for Facebook Tracking Pixel. Error @error', array('@error' => $e), WATCHDOG_ERROR, NULL);
      return form_set_error('facebook_tracking_pixel_tracking_code', 'Could not delete existing js files', NULL);
    };
    // Save the global js files for tracking pixel.
    if (!empty($fb_script[0]) && !empty($fb_noscript[0])) {
      $success = facebook_tracking_pixel_save_file($fb_script[0], 'fb_tkpx.js');
      if (!$success) {
        watchdog('facebook_tracking_pixel', 'Failed writing global JS files for Facebook Tracking Pixel.', array(), WATCHDOG_ERROR, NULL);
        return form_set_error('facebook_tracking_pixel_tracking_code', 'Could not save the JS file. Check the Drupal and PHP logs.', NULL);;
      }
      variable_set('facebook_tracking_pixel_noscripts', $fb_noscript[0]);
    }
  }
}

/**
 * Form builder for track by path
 */
function facebook_tracking_pixel_track_by_path_form($form, &$form_state) {
  $form = [];

  $form_state['storage']['facebook_tracking_pixel_tracking_codes_by_path'] = isset($form_state['storage']['facebook_tracking_pixel_tracking_codes_by_path']) ? $form_state['storage']['facebook_tracking_pixel_tracking_codes_by_path'] : 0;
  $form['intro_0'] = [
    '#markup' => '<p><h2>' . t('Facebook Event tracking via path') . '</h2></p>',
  ];

  $form['intro_1'] = [
    '#markup' => '<p>' . t('For more information regarding event tracking, read Facebook\'s') . ' ' . l('documentation.', 'https://www.facebook.com/business/help/402791146561655', [
        '#external' => TRUE,
        'attributes' => ['target' => '_blank']
      ]) . '</p>',
  ];


  $existing_codes = variable_get('facebook_tracking_pixel_tracking_codes_by_path', NULL);
  if (!empty($existing_codes['code']) && is_array($existing_codes['code'])) {
    // Number of existing codes.
    if (empty($form_state['num_names'])) {
      $form_state['num_names'] = count($existing_codes['code']);
    }
    foreach ($existing_codes['code'] as $key => $value) {
      $existing_codes_value[$key] = $value;
    }
  }


  $form['facebook_tracking_pixel_tracking_codes_by_path'] = [
    '#type' => 'fieldset',
    '#title' => t('Facebook Event Tracking per path'),
    '#description' => t('Designate the event to track and the path. Multiple events can be assigned to the same path. Checkout completion events are handled in another area of this module.'),
    // Set up the wrapper so that AJAX will be able to replace the fieldset.
    '#prefix' => '<div id="tracking-codes-wrapper-path">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
  ];

  // Build the fieldset with the proper number of names. We'll use
  // $form_state['num_names'] to determine the number of textfields to build.
  if (empty($form_state['num_names'])) {
    $form_state['num_names'] = 1;
  }
  for ($i = 0; $i < $form_state['num_names']; $i++) {
    $form['facebook_tracking_pixel_tracking_codes_by_path']['code'][$i]['path'] = [
      '#title' => t('Path Number #') . ' ' . $i,
      '#type' => 'textfield',
      '#description' => t('Drupal page path to track for this event.'),
    ];
    $form['facebook_tracking_pixel_tracking_codes_by_path']['code'][$i]['path']['#default_value'] = isset($existing_codes_value[$i]) ? $existing_codes_value[$i] : '';
    $options = facebook_tracking_pixel_events();
    foreach ($options as $key => $value) {
      $selectoptions[$key] = $value['name'];
    }
    $form['facebook_tracking_pixel_tracking_codes_by_path']['code'][$i]['event'] = [
      '#title' => t('Event Number #') . ' ' . $i,
      '#type' => 'select',
      '#options' => $selectoptions,
      '#description' => t('Select the event type.'),
    ];
    $form['facebook_tracking_pixel_tracking_codes_by_path']['code'][$i]['event']['#default_value'] = isset($existing_codes_value[$i]) ? $existing_codes_value[$i] : '';
  }
  $form['facebook_tracking_pixel_tracking_codes_by_path']['add_name'] = [
    '#type' => 'submit',
    '#value' => t('Add one more'),
    '#submit' => ['facebook_tracking_pixel_add_more_add_one'],
    '#ajax' => [
      'callback' => 'facebook_tracking_pixel_path_add_more_callback',
      'wrapper' => 'tracking-codes-wrapper-path',
    ],
  ];
  if ($form_state['num_names'] > 1) {
    $form['facebook_tracking_pixel_tracking_codes_by_path']['remove_name'] = [
      '#type' => 'submit',
      '#value' => t('Remove one'),
      '#submit' => ['facebook_tracking_pixel_add_more_remove_one'],
      '#ajax' => [
        'callback' => 'facebook_tracking_pixel_path_add_more_callback',
        'wrapper' => 'tracking-codes-wrapper-path',
      ],
    ];
  }

  // Add a custom submission handler.
  $form['#submit'][] = 'facebook_tracking_pixel_path_settings_submit';
  return system_settings_form($form);
}

/**
 * Callback for both ajax-enabled buttons for path tracking.
 *
 * Selects and returns the fieldset with the names in it.
 */
function facebook_tracking_pixel_path_add_more_callback($form, $form_state) {
  return $form['facebook_tracking_pixel_tracking_codes_by_path'];
}


/**
 * Implements hook_submit().
 * Settings form for tracking by path - submit callback.
 */
function facebook_tracking_pixel_path_settings_submit($form, &$form_state) {

  // Get path set in main part of module.
  $path = variable_get('facebook_tracking_pixel_path', 'public://facebook_tracking_pixel_path');

  // The data from Facebook has to be parsed to extract the script and the
  // noscript information. We use PHP DOM processing to make it easy.


  if (!empty($form_state['values']['facebook_tracking_pixel_tracking_codes_by_path']['code']) && is_array($form_state['values']['facebook_tracking_pixel_tracking_codes_by_path']['code'])) {
    $fb_event = [];
    $fb_path = [];
    foreach ($form_state['values']['facebook_tracking_pixel_tracking_codes_by_path']['code'] as $script) {
      !empty($script['event']) ? $fb_event[] = $script['event'] : '';
    }
    if (!empty($script['path'])) {
      $fb_path[] = $script['path'];
    }
    else {
      //form_set_error()
    }
  }
}
