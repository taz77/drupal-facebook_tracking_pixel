<?php

/**
 * @file facebook_tracking_pixel.admin.inc
 * Admin pages.
 */

/**
* Form builder.
*/
function facebook_tracking_pixel_settings_form($form, &$form_state) {
$form_state['storage']['tracking_codes'] = isset($form_state['storage']['tracking_codes']) ? $form_state['storage']['tracking_codes'] : 0;
$form['intro'] = array(
'#markkup' => t('Global Facebook Tracking Pixel Codes.')
);
$form['aggregation'] = array(
'#type' => 'fieldset',
'#description' => t('Aggregation settings for the javascripts. Default setting is to allow aggregation. Uncheck to disable aggregation of the tracking javascript codes.')
);
$form['aggregation']['facebook_tracking_pixel_aggregation'] = array(
'#type' => 'checkbox',
'#title' => 'JS Aggregation',
'#default_value' => variable_get('facebook_tracking_pixel_aggregation', TRUE),
);

$form['tracking_codes'] = array(
'#type' => 'container',
'#tree' => TRUE,
'#prefix' => '<div id="tracking_codes">',
  '#suffix' => '</div>',
);
if ($form_state['storage']['tracking_codes']) {
for ($i = 1; $i <= $form_state['storage']['tracking_codes']; $i++) {
$form['tracking_codes'][$i] = array(
'#type' => 'fieldset',
'#tree' => TRUE,
'#description' => t('Facebook tracking pixel javascript, pasted from Facebook.'),
);

$form['tracking_codes'][$i]['code'] = array(
'#title' => t('Code'),
'#type' => 'textarea',
);

}
}
$form['add_code'] = array(
'#type' => 'button',
'#value' => t('Add A Code'),
'#href' => '',
'#ajax' => array(
'callback' => 'facebook_tracking_pixel_ajax_add_code',
'wrapper' => 'tracking_codes',
),
);
$form_state['storage']['tracking_codes']++;
return system_settings_form($form);
}

/**
* AJAX call back for admin form to add more codes.
* @param $form
* @param $form_state
* @return mixed
*/
function facebook_tracking_pixel_ajax_add_code($form, $form_state) {
return $form['tracking_codes'];

}