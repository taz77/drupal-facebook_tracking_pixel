<?php

/**
 * @file facebook_tracking_pixel.admin.path.inc
 * CRUD forms for tracking path entries.
 */


/**
 * Form builder for tracking path add.
 */
function facebook_tracking_pixel_path_add_form($form, &$form_state) {
  $form['intro_0'] = [
    '#markup' => '<p>' . t('Global Facebook Tracking Pixel Codes.') . '</p>',
  ];

  $form['event_path'] = [
    '#title' => t('Path to track'),
    '#type' => 'textfield',
    '#size' => 24,
    '#maxlength' => 255,
    '#description' => t('The relative path to have this event to take place.'),
  ];

  $form['event_type'] = [
    '#title' => t('Event type'),
    '#type' => 'select',
    '#options' => facebook_tracking_pixel_events_options(),
    '#description' => t('Event type to track on the supplied path.'),
    '#default_value' => (isset($_GET['cid']) && isset($node->webform['components'][$_GET['cid']])) ? $node->webform['components'][$_GET['cid']]['type'] : 'textfield',
  ];

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('facebook_tracking_pixel_path_add_form_submit'),
  );
  $form['#validate'][] = 'facebook_tracking_pixel_path_add_validate';
  return $form;
}

/**
 * Validate handler for facebook_tracking_pixel_track_by_path_form() when adding a new event path
 */
function facebook_tracking_pixel_path_add_validate($form, &$form_state) {
  // Check that the entered component name is valid.
  if (drupal_strlen(trim($form_state['values']['event_path'])) <= 0) {
    form_set_error('event_path', t('When adding a new event, the path field is required.'));
  }
}


/**
 * Submit handler to add a new path based event.
 *
 * @param $form
 * @param $form_state
 */
function facebook_tracking_pixel_path_add_form_submit($form, &$form_state) {
  // @todo handle the items and write them to the files system and update the db
  $global_fb_tracking_path = variable_get('facebook_tracking_pixel_path', 'public://facebook_tracking_pixel');
  $subdir = variable_get('facebook_tracking_pixel_path_subdir', 'pathtracking');
  $uid = uniqid('', TRUE);
  // We have to verify the global JS file is already setup
  if (file_exists($global_fb_tracking_path . '/fb_tkpx.js')) {
    // Read the global file and append it.
    $filename = $global_fb_tracking_path . '/fb_tkpx.js';
    $handle = fopen($filename, 'r');
    $contents = fread($handle, filesize($filename));
    $path = $form_state['values']['event_path'];
    $type = $form_state['values']['event_type'];
    $fb_events = facebook_tracking_pixel_events();
    $event_type = $fb_events[$type];
    $contents .= "\r" . $event_type['code'];
    // Write out the new file for this tracking item.
    facebook_tracking_pixel_save_file($contents, $uid . '.js', $subdir);
    // Close the global file handler.
    fclose($handle);

    // Full path and filename.
    $event_js = $global_fb_tracking_path . '/' . $subdir . '/' . $uid;
    db_insert('facebook_tracking_pixel_events_path')
      ->fields(
        [
          'event_path' => $path,
          'event_type' => $type,
          'event_js_file' => $event_js,
          'event_uid' => $uid,
          'weight' => 0
        ]
      )
      ->execute();

  }
  else {
    // Return an error because there is no global JS file setup.
    drupal_set_message(t('Your addition could not be saved because there is no global pixel JS file. Please check the logs'), 'warning', FALSE);
  }

}

/**
 * Form builder for tracking path delete.
 */
function facebook_tracking_pixel_path_delete_form($form, &$form_state) {

}

/**
 * Form builder for tracking path edit.
 */
function facebook_tracking_pixel_path_edit_form($form, &$form_state) {

}
