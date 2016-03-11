#!/bin/bash

cd $DRUPAL_TI_DRUPAL_DIR
wget https://www.drupal.org/files/issues/drupal-session_destroy_return_bool-2460833-30.patch
git apply drupal-session_destroy_return_bool-2460833-30.patch
drush dl ctools-7.x-3.x-dev
drush dl commerce-7.x-1.11
drush en -y commerce_tax, commerce_price, commerce_cart, commerce_product, commerce_order, commerce