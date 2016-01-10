<?php

/**
 * @file facebook_tracking_pixel.inc
 * Functions for CRUD operations
 *
 * @author Brady Owens <info@fastglass.net>
 */


/**
 * Remove a base code by the FBID number.
 *
 * @param int $tid
 * @param int $fbid
 * @return bool
 */
function facebook_tracking_pixel_base_code_delete($tid, $fbid) {
  $status = TRUE;
  // Remove JS file.
  $filename = 'fb_tkpx.' . $fbid . '.js';
  $filedelete = facebook_tracking_pixel_delete_file($filename);
  if (!$filedelete) {
    $status = FALSE;
  }
  if (!$status) {
    drupal_set_message(t('Failed to delete base code file.'), 'error', FALSE);
  }
  // Remove base code from database.
  if (!db_delete('facebook_tracking_pixel_base_codes')
    ->condition('base_code_id', $tid, '=')
    ->execute()
  ) {
    $status = FALSE;
  }

  $pathcount = db_select('facebook_tracking_pixel_events_path', 'c')
    ->fields('c')
    ->condition('event_base_code_id', $tid, '=')
    ->countQuery()
    ->execute()
    ->fetchField();

  if ($pathcount > 0) {
    $subdir = variable_get('facebook_tracking_pixel_path_subdir', 'pathtracking');
    try {
      $deletepathuids = db_select('facebook_tracking_pixel_events_path', 'c')
        ->fields('c', ['event_uid'])
        ->condition('event_base_code_id', $tid, '=')
        ->execute()
        ->fetchAssoc();

      foreach ($deletepathuids as $item) {
        facebook_tracking_pixel_delete_file('fb_trk.' . $item . '.js', $subdir);
      }
    }
    catch (Exception $e) {
      watchdog('facebook_tracking_pixel', 'Failed to delete path items when deleting a base code :error.', [':error' => $e], WATCHDOG_ERROR, NULL);
      drupal_set_message('Could not save the JS file. Check the Drupal and PHP logs.', 'error', FALSE);
      $status = FALSE;
    }
    // Remove path tracking referencing this basecode.
    try {
      db_delete('facebook_tracking_pixel_events_path')
        ->condition('event_base_code_id', $tid, '=')
        ->execute();
    }
    catch (Exception $e) {
      watchdog('facebook_tracking_pixel', 'Failed to delete path items DB entries when deleting a base code :error.', [':error' => $e], WATCHDOG_ERROR, NULL);
      $status = FALSE;
    }
  }

  if (!$status) {
    drupal_set_message(t('Failed to remove base code from database.'), 'error', FALSE);
  }

  if (!$status) {
    drupal_set_message(t('Check your PHP server logs.'), 'error', FALSE);
  }
  else {
    drupal_set_message(t('Base code deleted.'), 'status', FALSE);
  }
  // Trigger reloading the CSS and JS file cache in AdvAgg, if available.
  if ($filedelete && module_exists('advagg')) {
    module_load_include('inc', 'advagg', 'advagg.cache');
    advagg_push_new_changes();
  }
  elseif ($filedelete) {
    // Clear JS cache.
    drupal_clear_js_cache();
  }
  return $status;
}


/**
 * Deletes all CSS & JavaScript from the file system or a single file.
 *
 * @param null $filename
 * @param null $subdir
 * @return bool
 */
function facebook_tracking_pixel_delete_file($filename = NULL, $subdir = NULL) {
  $path = variable_get('facebook_tracking_pixel_path', 'public://facebook_tracking_pixel');
  // Calling the function without a file name will delete everything.
  if ($filename == NULL) {
    if (!$files = file_scan_directory($path, '/.*\.js$/')) {
      foreach ($files as $key => $value) {
        file_unmanaged_delete($path . '/' . $key);
      }
      return TRUE;
    }
  }
  else {
    if (!empty($subdir)) {
      $deletepath = $path . '/' . $subdir . '/' . $filename;
    }
    else {
      $deletepath = $path . '/' . $filename;
    }
    file_unmanaged_delete($deletepath);
    return TRUE;
  }
  return FALSE;
}

/**
 * Saves JavaScript in the file system (but only if not empty).
 *
 * @param $data
 * @param $filename
 * @param null $subdir
 * @return bool|null|string
 */
function facebook_tracking_pixel_save_file($data, $filename, $subdir = NULL) {
  if (!drupal_strlen(trim($data))) {
    return FALSE;
  }
  $path = variable_get('facebook_tracking_pixel_path', 'public://facebook_tracking_pixel');
  if (!empty($subdir)) {
    $path = $path . '/' . $subdir;
  }
  file_prepare_directory($path, FILE_CREATE_DIRECTORY);
  $file_saved = file_unmanaged_save_data($data, $path . '/' . $filename, FILE_EXISTS_REPLACE);

  // Trigger reloading the CSS and JS file cache in AdvAgg, if available.
  if ($file_saved && module_exists('advagg')) {
    module_load_include('inc', 'advagg', 'advagg.cache');
    advagg_push_new_changes();
  }
  // Clear JS cache.
  drupal_clear_js_cache();
  return $file_saved;
}
