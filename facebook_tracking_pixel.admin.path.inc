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
    '#type' => 'textfield',
    '#size' => 24,
    '#maxlength' => 255,
    '#description' => t('The relative path to have this event to take place.'),
  ];

  $form['event_type'] = [
    '#type' => 'select',
    '#options' => facebook_tracking_pixel_events(),
    '#description' => t('Event type'),
    '#weight' => 3,
    '#default_value' => (isset($_GET['cid']) && isset($node->webform['components'][$_GET['cid']])) ? $node->webform['components'][$_GET['cid']]['type'] : 'textfield',
  ];

  // Add a custom submission handler.
  $form['#submit'][] = 'facebook_tracking_pixel_path_add_form_submit';
  return system_settings_form($form);
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
  // We have to verify the global JS file is already setup
  if (file_exists($global_fb_tracking_path . '/fb_tkpx.js')) {
    // Read the global file and append it.
    $filename = $global_fb_tracking_path . '/fb_tkpx.js';
    $handle = fopen($filename, 'r');
    $contents = fread($handle, filesize($filename));
    $path = $form_state['event_path'];
    $type = $form_state['event_type'];
    $fb_events = facebook_tracking_pixel_events();


    fclose($handle);

  }
  else {
    // Return an error because there is no global JS file setup.
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
