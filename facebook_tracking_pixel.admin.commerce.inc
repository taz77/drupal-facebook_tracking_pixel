<?php

/**
 * @file facebook_tracking_pixel.admin.commerce.inc
 * Commerce tracking capabilities admin forms and functions.
 *
 * @author Brady Owens <info@fastglass.net>
 */

/**
 * Form builder for commerce tracking.
 *
 * @param $form
 * @param $form_state
 * @return mixed
 */
function facebook_tracking_pixel_commerce_form($form, &$form_state) {

  $currencycodesraw = commerce_currencies();
  $currencycodes = [];
  foreach ($currencycodesraw as $key => $value) {
    $currencycodes[$key] = $value['name'] . ' (' . $key . ')';
  }
  if (!module_exists('commerce')) {
    $form['intro0'] = [
      '#markup' => t('This portion of the module is only compatible with Drupal Commerce.'),
    ];
    $form['intro1'] = [
      '#markup' => l('Download', 'https://www.drupal.org/project/commerce', ['external' => TRUE]) . ' ' . t('Drupal Commerce and install it to use these functions.'),
    ];
    return $form;
  }

  $form['intro0'] = [
    '#markup' => t('Enabliing this will track user registration for the entire site. To not track users added by admins, do not enable tracking for their roles.'),
  ];
  //Check to see if there are base codes.
  $resultcountbasecode = db_select('facebook_tracking_pixel_base_codes', 'c')
    ->fields('c')
    ->countQuery()
    ->execute()
    ->fetchField();
  if ($resultcountbasecode == 0) {
    $form['emptyset'] = [
      '#type' => 'hidden',
      '#value' => TRUE,
    ];
    $form['notice'] = [
      '#markup' => t('You must first set a base code for tracking.'),
    ];
    return $form;
  }
  else {
    // If we have results then build an array to use as an option drop down for
    // the path items.
    $resultbasecode = db_select('facebook_tracking_pixel_base_codes', 'c')
      ->fields('c')
      ->orderBy('weight')
      ->execute()
      ->fetchAllAssoc('base_code_id');
    $basecodes = [];
    foreach ($resultbasecode as $item) {
      $basecodes[$item->base_code_id] = $item->base_code_name;
    }
  }
  $enable = variable_get('facebook_tracking_pixel_commerce_tracking_enable', 0);
  $form['facebook_tracking_pixel_commerce_tracking_enable'] = [
    '#title' => t('Enable the tracking of Drupal Commerce sales.'),
    '#type' => 'checkbox',
    '#default_value' => $enable,
    '#ajax' => [
      'callback' => 'facebook_tracking_pixel_commerce_tracking_ajax',
      'wrapper' => 'replace-facebook-tracking-pixel-commerce-tracking-basecodes',
      'effect' => 'fade',
    ],
  ];
  $form['facebook_tracking_pixel_commerce_tracking_options'] = [
    '#type' => 'fieldset',
    '#title' => t('Enable Drupal Commerce tracking to set options.'),
    '#prefix' => '<div id="replace-facebook-tracking-pixel-commerce-tracking-basecodes">',
    '#suffix' => '</div>',
  ];
  if ((!empty($form_state['values']['facebook_tracking_pixel_commerce_tracking_enable']) && $form_state['values']['facebook_tracking_pixel_commerce_tracking_enable']) || $enable == 1) {
    $form['facebook_tracking_pixel_commerce_tracking_options']['#title'] = t('Commerce tracking options.');
    $form['facebook_tracking_pixel_commerce_tracking_options']['facebook_tracking_pixel_commerce_tracking'] = [
      '#type' => 'checkboxes',
      '#title' => t('Events to track'),
      '#options' => [
        'addpayment' => t('Add Payment'),
        'checkoutstart' => t('Checkout Start'),
        'addtocart' => t('Add to Cart'),
        'purchase' => t('Purchase'),
      ],
      '#ajax' => [
        'callback' => 'facebook_tracking_pixel_commerce_tracking_currency_codes_ajax',
        'wrapper' => 'replace-facebook-tracking-pixel-commerce-tracking-currency',
        'effect' => 'fade',
      ],
    ];
    $form['facebook_tracking_pixel_commerce_tracking_options']['facebook_tracking_pixel_commerce_tracking_basecode'] = [
      '#type' => 'select',
      '#title' => t('Facebook Base Tracking Code to Use'),
      '#options' => $basecodes,
      '#default_value' => variable_get('facebook_tracking_pixel_commerce_tracking_basecode', NULL),
    ];

  }
  $form['facebook_tracking_pixel_commerce_tracking_currency_selection'] = [
    '#markup' => '',
    '#prefix' => '<div id="replace-facebook-tracking-pixel-commerce-tracking-currency">',
    '#suffix' => '</div>',
  ];

  if ((!empty($form_state['values']['facebook_tracking_pixel_commerce_tracking']) && array_key_exists('purchase', array_filter($form_state['values']['facebook_tracking_pixel_commerce_tracking'])))) {
    unset($form['facebook_tracking_pixel_commerce_tracking_currency_selection']['#markup']);
    $form['facebook_tracking_pixel_commerce_tracking_currency_selection']['#type'] = 'select';
    $form['facebook_tracking_pixel_commerce_tracking_currency_selection']['#options'] = $currencycodes;
    $form['facebook_tracking_pixel_commerce_tracking_currency_selection']['#title'] = t('Currency Type');
  }
  $form['submit'] = [
    '#type' => 'submit',
    '#value' => 'Save configuration',
  ];

  return $form;
}

/**
 * AJAX callback to populate options for tracking.
 *
 * @param $form
 * @param $form_state
 * @return mixed
 */
function facebook_tracking_pixel_commerce_tracking_ajax($form, $form_state) {
  return $form['facebook_tracking_pixel_commerce_tracking_options'];
}

/**
 * AJAX callback to populate currency selection.
 *
 * @param $form
 * @param $form_state
 * @return mixed
 */
function facebook_tracking_pixel_commerce_tracking_currency_codes_ajax($form, $form_state) {
  return $form['facebook_tracking_pixel_commerce_tracking_currency_selection'];
}

/**
 * Submit handler for commerce tracking admin form.
 *
 * @param $form
 * @param $form_state
 */
function facebook_tracking_pixel_commerce_form_submit($form, $form_state) {
  // Sanitize variables for good measure.
  $enabled = check_plain($form_state['values']['facebook_tracking_pixel_commerce_tracking_enable']);
  !empty($enabled) ? variable_set('facebook_tracking_pixel_commerce_tracking_enable', $enabled) : variable_set('facebook_tracking_pixel_commerce_tracking_enable', 0);

  if (!empty($form_state['values']['facebook_tracking_pixel_commerce_tracking_basecode'])) {
    $basecodeid = check_plain($form_state['values']['facebook_tracking_pixel_commerce_tracking_basecode']);
    // This is saving the basecode ID number - not the basecode itself.
    variable_set('facebook_tracking_pixel_commerce_tracking_basecode', $basecodeid);
    $fbscriptjs = facebook_tracking_pixel_base_code_js();
    $eventtext = 'fbq(\'track\', "CompleteRegistration");';
    // This is the full JS code to save to the file system.
    $base_code_fbid = db_select('facebook_tracking_pixel_base_codes', 'c')
      ->fields('c', ['base_code_fbid'])
      ->condition('base_code_id', $basecodeid, '=')
      ->execute()
      ->fetchField();

    // @todo this cannot be file based since the value of shopping cart changes
    $data = $fbscriptjs[0] . $base_code_fbid . $fbscriptjs[1] . $eventtext;

  }
}
