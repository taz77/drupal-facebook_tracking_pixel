<?php

/**
 * @file facebook_tracking_pixel.admin.path.inc
 * CRUD forms for tracking path entries.
 *
 * @author Brady Owens <info@fastglass.net>
 */


/**
 * Form builder for track by path
 */
function facebook_tracking_pixel_track_by_path_form($form, &$form_state) {
  $form = [];
  $form['intro_0'] = [
    '#markup' => '<p><h2>' . t('Facebook Event tracking via path') . '</h2></p>',
  ];
  $form['intro_1'] = [
    '#markup' => '<p>' . t('For more information regarding event tracking, read Facebook\'s') . ' ' . l('documentation.', 'https://www.facebook.com/business/help/402791146561655', [
        '#external' => TRUE,
        'attributes' => ['target' => '_blank']
      ]) . '</p>',
  ];

  // Identify that the elements in 'example_items' are a collection, to
  // prevent Form API from flattening the array when submitted.
  $form['facebook_tracking_pixel_tracking_codes_by_path']['#tree'] = TRUE;

  // Fetch the example data from the database, ordered by weight ascending.
  //
  // This query excludes the last two tabledrag_example database rows, as
  // they are intended only for the 'parent/child' tabledrag examples.
  $result = db_query('SELECT * FROM {facebook_tracking_pixel_events_path} ORDER BY weight ASC');
  // Iterate through each database result.
  foreach ($result as $item) {
    $id = $item->event_id;
    $form['facebook_tracking_pixel_tracking_codes_by_path'][$id]['event_uid'] = [
      '#type' => 'hidden',
      '#default_value' => $item->event_uid,
    ];
    $form['facebook_tracking_pixel_tracking_codes_by_path'][$id]['weight'] = [
      '#type' => 'textfield',
      '#size' => 4,
      '#title' => t('Weight'),
      '#default_value' => $item->weight,
    ];
  }
  kpr($form);

  $form['add']['event_path'] = array(
    '#type' => 'textfield',
    '#size' => 24,
    '#maxlength' => 255,
  );

  $form['add']['event_type'] = array(
    '#type' => 'select',
    '#options' => facebook_tracking_pixel_events_options(),
    '#weight' => 3,
    '#default_value' => (isset($_GET['cid']) && isset($node->webform['components'][$_GET['cid']])) ? $node->webform['components'][$_GET['cid']]['type'] : 'textfield',
  );
  $form['add']['weight'] = array(
    '#type' => 'textfield',
    '#size' => 4,
  );
  /*
    if (isset($_GET['cid']) && isset($node->webform['components'][$_GET['cid']])) {
      // Make the new component appear by default directly after the one that was
      // just added.
      $form['add']['weight']['#default_value'] = $node->webform['components'][$_GET['cid']]['weight'] + 1;
      foreach (array_keys($node->webform['components']) as $cid) {
        // Adjust all later components also, to make sure none of them have the
        // same weight as the new component.
        if ($form['components'][$cid]['weight']['#default_value'] >= $form['add']['weight']['#default_value']) {
          $form['components'][$cid]['weight']['#default_value']++;
        }
      }
    }
    else {
      // If no component was just added, the new component should appear by
      // default at the end of the list.
      $form['add']['weight']['#default_value'] = isset($max_weight) ? $max_weight + 1 : 0;
    }
  */
  $form['add']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
    '#weight' => 45,
    '#validate' => array(
      'facebook_tracking_pixel_path_add_validate',
      'webform_components_form_validate'
    ),
    '#submit' => array('facebook_tracking_pixel_path_add_form'),
  );


  // Now we add our submit button, for submitting the form results.
  //
  // The 'actions' wrapper used here isn't strictly necessary for tabledrag,
  // but is included as a Form API recommended practice.
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Changes')
  );
  return $form;
}

/**
 * Theme callback for the facebook_tracking_pixel_track_by_path_form form.
 */
function theme_facebook_tracking_pixel_track_by_path_form($variables) {
  $form = $variables['form'];


  // We now define the table header values.  Ensure that the 'header' count
  // matches the final column count for your table.
  //$header = array(t('Event Type'), t('Event Path'), t('Weight'));
  $header = [
    t('Event Type'),
    t('Event Path'),
    t('Weight'),
    [
      'data' => t('Operations'),
      'colspan' => 3,
    ],
  ];


  // Initialize the variable which will store our table rows.
  $rows = [];


  // Iterate over each element in our $form['example_items'] array.
  foreach (element_children($form['facebook_tracking_pixel_tracking_codes_by_path']) as $id) {

    // Before we add our 'weight' column to the row, we need to give the
    // element a custom class so that it can be identified in the
    // drupal_add_tabledrag call.
    //
    // This could also have been done during the form declaration by adding
    // '#attributes' => array('class' => 'example-item-weight'),
    // directy to the 'weight' element in tabledrag_example_simple_form().
    $form['facebook_tracking_pixel_tracking_codes_by_path'][$id]['weight']['#attributes']['class'] = ['facebook-tracking-pixel-path-weight'];

    // We are now ready to add each element of our $form data to the $rows
    // array, so that they end up as individual table cells when rendered
    // in the final table.  We run each element through the drupal_render()
    // function to generate the final html markup for that element.
    $rows[] = [
      'data' => [
        // Add our 'event_type' column.
        drupal_render($form['facebook_tracking_pixel_tracking_codes_by_path'][$id]['event_type']),
        // Add our 'event_path' column.
        drupal_render($form['facebook_tracking_pixel_tracking_codes_by_path'][$id]['event_path']),
        // Add our 'weight' column.
        drupal_render($form['facebook_tracking_pixel_tracking_codes_by_path'][$id]['weight']),
        // Operations column for editing and deleting.
        l(t('Edit'), 'admin/config/system/facebook_tracking_pixel/path/edit/' . $node->nid . '/webform/components/', array('query' => drupal_get_destination())),
        l(t('Delete'), 'admin/config/system/facebook_tracking_pixel/path/delete/' . $node->nid . '/webform/components/' . '/delete', array('query' => drupal_get_destination())),

      ],
      // To support the tabledrag behaviour, we need to assign each row of the
      // table a class attribute of 'draggable'. This will add the 'draggable'
      // class to the <tr> element for that row when the final table is
      // rendered.
      'class' => ['draggable'],
    ];
  }


  // We also need to pass the drupal_add_tabledrag() function an id which will
  // be used to identify the <table> element containing our tabledrag form.
  // Because an element's 'id' should be unique on a page, make sure the value
  // you select is NOT the same as the form ID used in your form declaration.
  $table_id = 'facebook-tracking-pixel-path-items-table';

  // We can render our tabledrag table for output.
  $output = theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => $table_id),
  ));

  // And then render any remaining form elements (such as our submit button).
  $output .= drupal_render_children($form);

  // We now call the drupal_add_tabledrag() function in order to add the
  // tabledrag.js goodness onto our page.
  //
  // For a basic sortable table, we need to pass it:
  // - the $table_id of our <table> element,
  // - the $action to be performed on our form items ('order'),
  // - a string describing where $action should be applied ('siblings'),
  // - and the class of the element containing our 'weight' element.
  drupal_add_tabledrag($table_id, 'order', 'sibling', 'example-item-weight');


  $output = '';
  $output .= theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => 'webform-components')
  ));
  $output .= drupal_render_children($form);

  return $output;
}


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
    $contents .= $event_type['code'];
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
    drupal_set_message(t('Tracking entry added.'), 'status', FALSE);
  }
  else {
    // Return an error because there is no global JS file setup.
    drupal_set_message(t('Your addition could not be saved because there is no global pixel JS file. Please check the logs'), 'warning', FALSE);
  }

}

/**
 * Form builder for tracking path delete.
 */
function facebook_tracking_pixel_path_delete_form($form, &$form_state, $uid) {
  // Remove the file from storage.
  // Delete the entry from the database.
  $status = db_delete('facebook_tracking_pixel_events_path')
    ->condition('event_uid', $uid, '=')
    ->execute();
}

/**
 * Form builder for tracking path edit.
 */
function facebook_tracking_pixel_path_edit_form($form, &$form_state, $uid) {

  $result = db_select('facebook_tracking_pixel_events_path', 'e')
    ->condition('event_uid', $uid, '=')
    ->fields('e')
    ->execute()
    ->fetchAssoc();

}


/**
 * Submit handler
 */
function STUFFFFFFFFfacebook_tracking_pixel_path_add_form_submit($form, &$form_state) {
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

  $fb_script_src = [
    '<script>
    !function(f,b,e,v,n,t,s){if(f.fbq)return;n=f.fbq=function(){n.callMethod?
    n.callMethod.apply(n,arguments):n.queue.push(arguments)};if(!f._fbq)f._fbq=n;
    n.push=n;n.loaded=!0;n.version=\'2.0\';n.queue=[];t=b.createElement(e);t.async=!0;
    t.src=v;s=b.getElementsByTagName(e)[0];s.parentNode.insertBefore(t,s)}(window,
    document,\'script\',\'//connect.facebook.net/en_US/fbevents.js\');',
    '</script>',
  ];
  $fb_noscript_src = [
    '<noscript><img height="1" width="1" style="display:none"
    src="https://www.facebook.com/tr?id=',
    '&ev=PageView&noscript=1"/></noscript>',
  ];

  if (!empty($form_state['values']['facebook_tracking_pixel_tracking_codes']['code'])) {


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
