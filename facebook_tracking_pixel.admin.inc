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

function facebook_tracking_pixel_settings_form($form_state) {
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
    '#validate' => ['facebook_tracking_pixel_settings_form_validate'],
  ];

  return system_settings_form($form);
}

function facebook_tracking_pixel_settings_form_validate($form, &$form_state) {
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
    '#submit' => ['facebook_tracking_pixel_base_codes_add_form_submit'],
    '#validate' => ['facebook_tracking_pixel_base_codes_add_fbid_validate'],
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
 * Validation function to make sure the FB Id is an integer.
 * @param $form
 * @param $form_state
 */
function facebook_tracking_pixel_base_codes_add_fbid_validate($form, &$form_state) {
  // Check that the entered component name is valid.
  if (!is_int($form_state['values']['add']['fbid'])) {
    form_error($form['add']['fbid'], t('Facebook ID numbers are numbers only.'));
  }
}

/**
 * Submit handler for the new tracking id form.
 * @param $form
 * @param $form_state
 * @return array
 */
function facebook_tracking_pixel_base_codes_add_form_submit($form, &$form_state) {
  /*
    t('Name'),
      t('Facebook ID Number'),
      t('Weight'),*/


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

  if (!empty($form_state['values']['add']['name']) && !empty($form_state['values']['add']['fbid'])) {
    // Construct our tracking codes.
    $fb_base_code = $fb_script_src[0] . $form_state['values']['add']['fbid'] . $fb_script_src[1];
    $fb_base_code_nojs = $fb_noscript_src[0] . $form_state['values']['add']['fbid'] . $fb_noscript_src[1];

    /* Remove existing file, if present.
    try {
      facebook_tracking_pixel_delete_file();
    }
    catch (Exception $e) {
      watchdog('facebook_tracking_pixel', 'Failed deleting global JS files for Facebook Tracking Pixel. Error @error', array('@error' => $e), WATCHDOG_ERROR, NULL);
      return form_set_error('facebook_tracking_pixel_tracking_code', 'Could not delete existing js files', NULL);
    };*/

    // Save the global js files for tracking pixel.
    if (!empty($fb_base_code) && !empty($fb_noscript[0])) {
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
  $sql = 'TRUNCATE {facebook_tracking_pixel_base_codes}';
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
