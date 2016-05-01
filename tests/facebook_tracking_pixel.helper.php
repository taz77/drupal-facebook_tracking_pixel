<?php

/**
 * @file
 * Contains tests for the Facebook Tracking Pixel module.
 */

namespace facebookTrackingPixel;

class FacebookTrackingPixelTestHelper {

  /**
   * Enable tracking for all roles.
   */
  public function enable_tracking_all_roles() {
    // Turn on tracking for roles.
    variable_set('facebook_tracking_pixel_roles_administrator', 1);
    variable_set('facebook_tracking_pixel_roles_anonymous_user', 1);
    variable_set('facebook_tracking_pixel_roles_authenticated_user', 1);
  }

  /**
   * Enable tracking for only the testing role.
   */

  public function enable_tracking_testing_role(){
    // Inverse the visibility.
    variable_set('facebook_tracking_pixel_visibility_roles', 1);
    variable_set('facebook_tracking_pixel_roles_fb_pixel_tester', 1);
    variable_set('facebook_tracking_pixel_roles_administrator', 0);
    variable_set('facebook_tracking_pixel_roles_anonymous_user', 0);
    variable_set('facebook_tracking_pixel_roles_authenticated_user', 0);
  }

}

