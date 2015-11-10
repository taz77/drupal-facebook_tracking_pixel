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
 * Implements hook_submit().
 *
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
  $result = db_query('SELECT event_id, event_path, event_type, event_js_file, weight FROM {facebook_tracking_pixel_events_path} ORDER BY weight ASC');
  // Iterate through each database result.
  foreach ($result as $item) {

    // Create a form entry for this item.
    //
    // Each entry will be an array using the the unique id for that item as
    // the array key, and an array of table row data as the value.
    $form['facebook_tracking_pixel_tracking_codes_by_path'][$item->event_id] = array(

      // We'll use a form element of type '#markup' to display the item name.
      'name' => [
        '#markup' => check_plain($item->event_type),
      ],

      // We'll use a form element of type '#textfield' to display the item
      // description, which will allow the value to be changed via the form.
      // We limit the input to 255 characters, which is the limit we set on
      // the database field.
      'path' => [
        '#type' => 'textfield',
        '#default_value' => check_plain($item->event_path),
        '#size' => 20,
        '#maxlength' => 255,
      ],

      // The 'weight' field will be manipulated as we move the items around in
      // the table using the tabledrag activity.  We use the 'weight' element
      // defined in Drupal's Form API.
      'weight' => [
        '#type' => 'weight',
        '#title' => t('Weight'),
        '#default_value' => $item->weight,
        '#delta' => 10,
        '#title_display' => 'invisible',
      ],
      'edit' => [

      ],
      'delete' => [

      ],
    );

  }

  $form['add']['event_path'] = array(
    '#type' => 'textfield',
    '#size' => 24,
    '#maxlength' => 255,
  );

  $form['add']['event_type'] = array(
    '#type' => 'select',
    '#options' => facebook_tracking_pixel_events(),
    '#weight' => 3,
    '#default_value' => (isset($_GET['cid']) && isset($node->webform['components'][$_GET['cid']])) ? $node->webform['components'][$_GET['cid']]['type'] : 'textfield',
  );
  $form['add']['weight'] = array(
    '#type' => 'textfield',
    '#size' => 4,
    '#delta' => count($node->webform['components']) > 10 ? count($node->webform['components']) : 10,
  );

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

  $form['add']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
    '#weight' => 45,
    '#validate' => array('webform_components_form_add_validate', 'webform_components_form_validate'),
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
 * Theme callback for the tabledrag_example_simple_form form.
 */
function theme_facebook_tracking_pixel_track_by_path_form($variables) {
  $form = $variables['form'];

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
        // Add our 'name' column.
        drupal_render($form['facebook_tracking_pixel_tracking_codes_by_path'][$id]['event_type']),
        // Add our 'description' column.
        drupal_render($form['facebook_tracking_pixel_tracking_codes_by_path'][$id]['event_path']),
        // Add our 'weight' column.
        drupal_render($form['facebook_tracking_pixel_tracking_codes_by_path'][$id]['weight']),
        l(t('Edit'), 'node/' . $node->nid . '/webform/components/', array('query' => drupal_get_destination())),
        l(t('Delete'), 'node/' . $node->nid . '/webform/components/' . '/delete', array('query' => drupal_get_destination())),

      ],
      // To support the tabledrag behaviour, we need to assign each row of the
      // table a class attribute of 'draggable'. This will add the 'draggable'
      // class to the <tr> element for that row when the final table is
      // rendered.
      'class' => ['draggable'],
    ];
  }

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

  return $output;
}
