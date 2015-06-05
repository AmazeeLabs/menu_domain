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
A pane (created from a Menu Domain content type plugin) will select a menu automatically basing on the menu machine name suffixes and the domain prefixes (marked as bold in the example table). The base menus are used for the default domain.'),
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
    foreach ($base_menu_names as $base_menu_name) {
      $header[] = $base_menu_name;
    }
    $rows = array();
    foreach ($domains as $domain) {
      $row = array();
      @list($domain_prefix) = explode('.', $domain['subdomain']);
      $row[] = $domain['sitename'] . ' (' . ($domain['is_default'] ? t('default') : $domain_prefix) . ')';
      foreach ($base_menu_names as $base_menu_name) {
        $menu_name = _menu_domain_get_menu_name($base_menu_name, $domain);
        if ($menu_name) {
          $row[] = l($menus[$menu_name], "admin/structure/menu/manage/$menu_name") . " ($menu_name)";
        }
        else {
          if (module_exists('menu_clone')) {
            $links = array();
            foreach ($domains as $_domain) {
              $_menu_name = _menu_domain_get_menu_name($base_menu_name, $_domain);
              if ($_menu_name) {
                $links[] = array(
                  'title' => t('Clone from "@menu"', array('@menu' => $menus[$_menu_name])),
                  'href' => "admin/structure/menu/menu-domain/quick-clone/$_menu_name/$domain_prefix",
                );
              }
            }
            $row[] = theme('ctools_dropdown', array('title' => t('Quick clone'), 'links' => $links));
          }
          else {
            $row[] = '';
          }
        }
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

/**
 * Clones a menu.
 *
 * @param string $source_menu_name
 *   The source menu machine name.
 * @param string $target_domain_prefix
 *   The target domain prefix which will be used as a target menu suffix.
 */
function menu_domain_quick_clone($source_menu_name, $target_domain_prefix) {
  $menus = menu_get_menus();

  // Prepare the base name/title for the target menu.
  $source_menu_title = $menus[$source_menu_name];
  $source_menu_name_parts = explode('-', $source_menu_name);
  $source_menu_name_suffix = array_pop($source_menu_name_parts);
  $source_menu_title_parts = explode(' ', $source_menu_title);
  $source_menu_title_suffix = array_pop($source_menu_title_parts);
  if (strtoupper($source_menu_name_suffix) == $source_menu_title_suffix) {
    $base_menu_name = implode('-', $source_menu_name_parts);
    $base_menu_title = implode(' ', $source_menu_title_parts);
  }
  else {
    $base_menu_name = $source_menu_name;
    $base_menu_title = $source_menu_title;
  }
  if (strpos($base_menu_name, 'menu-') === 0) {
    $base_menu_name = substr($base_menu_name, 5);
  }
  $target_menu_name = $base_menu_name . '-' . $target_domain_prefix;
  $target_menu_title = $base_menu_title . ' ' . strtoupper($target_domain_prefix);

  // Prepare the clone form (from menu_clone module).
  module_load_include('admin.inc', 'menu_clone', 'includes/menu_clone');
  $form_state = array();
  $form_state['build_info']['args'][] = menu_clone_load($source_menu_name);
  $form = drupal_build_form('menu_clone_clone_form', $form_state);
  form_get_cache($form['#build_id'], $form_state);

  // Setup values and submit the form.
  $form_state['values']['menu_name'] = $target_menu_name;
  $form_state['values']['title'] = $target_menu_title;
  $form_state['values']['edit_menu']['clone_localization'] = 1;
  $form_state['input'] = $form_state['values'];
  menu_clone_clone_form_submit($form, $form_state);

  drupal_set_message(t('The !target_menu has been successfully cloned from the !source_menu.', array(
    '!target_menu' => l($target_menu_title, "admin/structure/menu/manage/menu-$target_menu_name"),
    '!source_menu' => l($source_menu_title, "admin/structure/menu/manage/$source_menu_name"),
  )));
  drupal_goto('admin/structure/menu/menu-domain');
}
