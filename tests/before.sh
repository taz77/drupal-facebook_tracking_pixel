#!/bin/bash

cd $DRUPAL_TI_DRUPAL_DIR
wget https://www.drupal.org/files/issues/drupal-session_destroy_return_bool-2460833-30.patch
git apply drupal-session_destroy_return_bool-2460833-30.patch