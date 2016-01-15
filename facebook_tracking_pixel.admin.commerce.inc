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
function facebook_tracking_pixel_commerce_form($form, &$form_state){

  if(!module_exists('commerce')){
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
      $basecodes[$item->base_code_id] = $item->base_code_fbid;
    }
  }
  $enable = variable_get('facebook_tracking_pixel_user_registration_enable', 0);
  $form['facebook_tracking_pixel_user_registration_enable'] = [
    '#title' => t('Enable the tracking of user registrations'),
    '#type' => 'checkbox',
    '#default_value' => $enable,
    '#ajax' => [
      'callback' => 'facebook_tracking_pixel_user_registration_ajax',
      'wrapper' => 'replace-facebook-tracking-pixel-user-registration-basecodes',
      'effect' => 'fade',
    ],
  ];
  $form['facebook_tracking_pixel_user_registration_basecode'] = [
    '#markup' => t('Enable user tracking to set basecode'),
    // '#default_value' => variable_get('freshdesk_widget_popup_type_attribute'),
    '#prefix' => '<div id="replace-facebook-tracking-pixel-user-registration-basecodes">',
    '#suffix' => '</div>',
  ];
  if ((!empty($form_state['values']['facebook_tracking_pixel_user_registration_enable']) && $form_state['values']['facebook_tracking_pixel_user_registration_enable']) || $enable == 1 ) {
    unset($form['facebook_tracking_pixel_user_registration_basecode']['#options']);
    unset($form['facebook_tracking_pixel_user_registration_basecode']['#type']);
    unset($form['facebook_tracking_pixel_user_registration_basecode']['#markup']);
    $form['facebook_tracking_pixel_user_registration_basecode']['#type'] = 'select';
    $form['facebook_tracking_pixel_user_registration_basecode']['#title'] = t('Facebook Base Tracking Code to Use');
    $form['facebook_tracking_pixel_user_registration_basecode']['#options'] = $basecodes;
    $form['facebook_tracking_pixel_user_registration_basecode']['#default_value'] = variable_get('facebook_tracking_pixel_user_registration_basecode', NULL);
  }
  $form['submit'] = [
    '#type' => 'submit',
    '#value' => 'Save configuration',
  ];

  return $form;



}

