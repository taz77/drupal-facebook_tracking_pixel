<?php

/**
 * @file facebook_tracking_pixel.admin.inc
 * Admin pages.
 *
 * @author Brady Owens <info@fastglass.net>
 */

/**
 * Form builder for core modules settings.
 */

function facebook_tracking_pixel_settings_form($form_state){
  $form = [];
  $form['intro_0'] = [
    '#markup' => '<p>' . t('Global Facebook Tracking Pixel Codes.') . '</p>',
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
    '#default_value' => variable_get('facebook_tracking_pixel_path', 'public://facebook_tracking_pixel'),
    '#description' => t('The subdirectory of the system files directory where Facebook Tracking Pixel will store its files. Note: changing this path will cause the old path to move to the new path, overwriting the new path, if it exists.'),
  ];

  return system_settings_form($form);
}

/**
 * Form builder for base tracking codes.
 */
function facebook_tracking_pixel_base_codes_form($form_state) {

  $result = db_select('facebook_tracking_pixel_base_codes', 'c')
    ->fields('c')
    ->orderBy('weight')
    ->execute()
    ->fetchAllAssoc('base_code_id');

  $resultcount = db_select('facebook_tracking_pixel_base_codes', 'c')
    ->fields('c')
    ->countQuery()
    ->execute()
    ->fetchField();

  $form['base_codes']['#tree'] = TRUE;
  // Iterate through each database result.
  if ($resultcount > 0) {
    foreach ($result as $item) {
      $form['base_codes'][$item->base_code_id] = [
        'bcid' => [
          '#type' => 'hidden',
          '#value' => $item->base_code_id,
        ],
        'name' => [
          '#type' => 'textfield',
          '#default_value' => check_plain($item->base_code_name),
          '#size' => 20,
          '#maxlength' => 255,
        ],
        'fbid' => [
          '#type' => 'textfield',
          '#default_value' => check_plain($item->base_code_fbid),
          '#size' => 20,
          '#maxlength' => 255,
        ],
        'weight' => [
          '#type' => 'weight',
          '#default_value' => $item->weight,
          '#delta' => 10,
          '#title_display' => 'invisible',
          '#attributes' => [
            'class' => ['base-codes-weight'],
          ]
        ],
      ];
      // Determine the maximum weight value to use below for the add line.
      if (!isset($max_weight) || $item->weight > $max_weight) {
        $max_weight = $item->weight;
      }
    }
  }

  $form['add']['name'] = [
    '#type' => 'textfield',
    '#size' => 24,
    '#maxlength' => 255,
  ];
  $form['add']['fbid'] = [
    '#type' => 'textfield',
    '#default_value' => '',
    '#size' => 20,
    '#maxlength' => 255,
  ];
  $form['add']['weight'] = [
    '#type' => 'weight',
    '#default_value' => 0,
    '#delta' => $resultcount > 10 ? $resultcount : 10,
    '#title_display' => 'invisible',
  ];
  // @todo this does not work yet.
  if (isset($_GET['bcid'])) {
    // Make the new component appear by default directly after the one that was
    // just added.
    $form['add']['weight']['#default_value'] = $result[$_GET['bcid']]->weight + 1;
    foreach ($result as $cid) {
      // Adjust all later components also, to make sure none of them have the
      // same weight as the new component.
      if ($form['base_codes'][$cid]['weight']['#default_value'] >= $form['add']['weight']['#default_value']) {
        $form['base_codes'][$cid]['weight']['#default_value']++;
      }
    }
  }
  else {
    // If no component was just added, the new component should appear by
    // default at the end of the list.
    $form['add']['weight']['#default_value'] = isset($max_weight) ? $max_weight + 1 : 0;
  }

  $form['add']['add'] = [
    '#type' => 'submit',
    '#value' => t('Add'),
    '#weight' => 45,
    '#validate' => [
      'webform_components_form_add_validate',
      'webform_components_form_validate'
    ],
    '#submit' => ['facebook_tracking_pixel_settings_submit'],
  ];

  $form['actions'] = ['#type' => 'actions'];
  $form['actions']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 45,
  ];
  return $form;
}

/**
 * Theme callback for the base code settings form.
 *
 * @param $variables
 * @return string
 */
function theme_facebook_tracking_pixel_base_codes_form($variables) {
  $form = $variables['form'];

  // Initialize the variable to hold the rows of the table.
  $rows = [];
  // Table header.
  $header = [
    t('Name'),
    t('Facebook ID Number'),
    t('Weight'),
    ['data' => t('Operations'), 'colspan' => 2]
  ];

  // Add a row containing form elements for a new item.
  unset($form['add']['name']['#title'], $form['add_type']['#description']);
  $form['add']['name']['#attributes']['rel'] = t('New Tracking ID name');
  $form['add']['name']['#attributes']['class'] = array('base-code-name-default-value');
  $form['add']['fbid']['#attributes']['class'] = array('base-code-fbid');
  $form['add']['weight']['#attributes']['class'] = array('base-code-weight');
  $row_data = array(
    drupal_render($form['add']['name']),
    drupal_render($form['add']['fbid']),
    drupal_render($form['add']['bcid']) . drupal_render($form['add']['weight']),
    array('colspan' => 2, 'data' => drupal_render($form['add']['add'])),
  );
  $add_form = array(
    'data' => $row_data,
    'class' => array('draggable', 'base-code-add-form')
  );
  $form_rendered = FALSE;
  // Iterate over each element in our $form['base_codes'] array.
  foreach (element_children($form['base_codes']) as $id) {
    $rows[] = [
      'data' => [
        // Add our 'name' column.
        drupal_render($form['base_codes'][$id]['name']),
        // Add our 'description' column.
        drupal_render($form['base_codes'][$id]['fbid']),
        // Add our 'weight' column.
        drupal_render($form['base_codes'][$id]['weight']),
        l(t('Edit'), 'admin/config/system/facebook_tracking_pixel/path/edit/' . $form['base_codes'][$id]['bcid']['#value'], array('query' => drupal_get_destination())),
        l(t('Delete'), 'admin/config/system/facebook_tracking_pixel/path/delete/' . $form['base_codes'][$id]['bcid']['#value'] . '/delete', array('query' => drupal_get_destination())),
      ],
      // To support the tabledrag behaviour, we need to assign each row of the
      // table a class attribute of 'draggable'. This will add the 'draggable'
      // class to the <tr> element for that row when the final table is
      // rendered.
      'class' => ['draggable'],
    ];

  }
  // Append the add form if not already printed.
  if ($add_form) {
    $rows[] = $add_form;
  }

  $output = '';
  $output .= theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => 'facebook-base-tracking-ids')
  ));
  $output .= drupal_render_children($form);
  drupal_add_tabledrag('facebook-base-tracking-ids', 'order', 'sibling', 'base-codes-weight');
  return $output;
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
 * Form builder for purge feature.
 */
function facebook_tracking_pixel_form_purge($form, &$form_state) {
  $form['intro_0'] = [
    '#markup' => '<p>' . t('Destroy all codes, records, and files.') . '</p>',
  ];
  $form['intro_1'] = [
    '#markup' => '<p>' . t('This will purge the file system deleting all files and directories and purge the database of all configured paths. Use this function with caution.') . '</p>',
  ];
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Purge All'),
    '#submit' => array('facebook_tracking_pixel_form_purge_submit'),
  );
  return $form;
}

/**
 * Submit handler for purge function of module.
 */

function facebook_tracking_pixel_form_purge_submit() {
  $status = TRUE;
  $sql = 'TRUNCATE {facebook_tracking_pixel_events_path}';
  if (!db_query($sql)) {
    $status = FALSE;
  }
  $path = variable_get('facebook_tracking_pixel_path', 'public://facebook_tracking_pixel');
  if (!file_unmanaged_delete_recursive($path)) {
    $status = FALSE;
  }
  if (!$status) {
    drupal_set_message(t('Purging failed check your PHP logs on your server.'), 'error', FALSE);
  }
  else {
    drupal_set_message(t('Purging complete'), 'status', FALSE);
  }
  return $status;
}
