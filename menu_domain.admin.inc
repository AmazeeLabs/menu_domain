<?php

/**
 * @file
 * Administration forms.
 */

/**
 * Menu Domain settings form callback.
 */
function menu_domain_admin_form($form, &$form_state) {
  $base_menu_names = variable_get('menu_domain_base_menu_names', array());
  $form['instructions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Instructions'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    'markup' => array(
      '#markup' => t('The Menu Domain module helps to build a menu-per-domain system. It provides ctools content type plugins which could be added to site via panels/panelizer. An example:
<pre><b>Menu title       Menu machine name     Domain name     Domain</b>
Main menu        menu-main-menu        Global          example.com
Main menu CH     menu-main-menu-<b>ch</b>     Switzerland     <b>ch</b>.example.com
Main menu FR     menu-main-menu-<b>fr</b>     France          <b>fr</b>.example.com
Footer menu      menu-footer-menu      Global          example.com
Footer menu CH   menu-footer-menu-<b>ch</b>   Switzerland     <b>ch</b>.example.com
Footer menu FR   menu-footer-menu-<b>fr</b>   France          <b>fr</b>.example.com</pre>
To make this work, you need to select "Main menu" and "Footer menu" as the base menus. After that you\'ll find the content type plugins in the Menu Domain group.<br />
The selection will happen automatically basing on the menu machine name suffixes and the domain prefixes (marked as bold in the example table).'),
    ),
  );

  $options = array();
  foreach (menu_get_menus() as $menu_name => $menu_title) {
    $options[$menu_name] = "$menu_title ($menu_name)";
  }
  $form['base_menu_names'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Base menus'),
    '#options' => $options,
    '#default_value' => $base_menu_names,
  );

  $form['status'] = array(
    '#type' => 'fieldset',
    '#title' => t('Status'),
    '#collapsible' => TRUE,
  );
  if (empty($base_menu_names)) {
    $form['status']['markup']['#markup'] = t('No base menus defined.');
  }
  else {
    $menus = menu_get_menus();
    $domains = domain_domains();
    $header = array();
    $header[] = '';
    foreach ($base_menu_names as $menu_name_base) {
      $header[] = $menu_name_base;
    }
    $rows = array();
    foreach ($domains as $domain) {
      $row = array();
      @list($prefix) = explode('.', $domain['subdomain']);
      $row[] = $domain['sitename'] . ' (' . ($domain['is_default'] ? t('default') : $prefix) . ')';
      foreach ($base_menu_names as $menu_name_base) {
        $menu_name = _menu_domain_get_menu_name($menu_name_base, $domain);
        $row[] = $menu_name ? "{$menus[$menu_name]} ($menu_name)" : '';
      }
      $rows[] = $row;
    }
    $form['status']['table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Submit handler for the settings form.
 */
function menu_domain_admin_form_submit(&$form, &$form_state) {
  variable_set('menu_domain_base_menu_names', array_filter($form_state['values']['base_menu_names']));
  drupal_set_message(t('The configuration options have been saved.'));
}
