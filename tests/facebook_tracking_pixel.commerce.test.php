<?php

/**
 * @file
 * Contains tests for the Facebook Tracking Pixel module commerce testing.
 */

/**
 * Test case.
 */
class FacebookTrackingPixelTestCaseCommerce extends FacebookTrackingPixelTestCase {

  /**
   * {@inheritdoc}
   */
  public static function getInfo() {
    return [
      'name' => 'Commerce Test',
      'description' => 'Test the Drupal Commerce features.',
      'group' => 'Facebook Tracking Pixel',
    ];
  }

  /**
   * {@inheritdoc}
   */
  function setUp() {
    // Call the parent with an array of modules to enable for the test.
    $modules[] = 'commerce';
    parent::setUp($modules);

  }


}