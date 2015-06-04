<?php

/**
 * @file
 * Defined "Menu Domain" content type plugin.
 */

$plugin = array(
  'title' => t('Menu Domain'),
  'content type' => '_menu_domain_content_type',
);

/**
 * Content type callback.
 */
function _menu_domain_content_type($subtype_id) {
  $subtypes = menu_domain_menu_domain_content_type_content_types();
  if (isset($subtypes[$subtype_id])) {
    return $subtypes[$subtype_id];
  }
  return NULL;
}

/**
 * All content types callback.
 */
function menu_domain_menu_domain_content_type_content_types() {
  $types =& drupal_static(__FUNCTION__);
  if (isset($types)) {
    return $types;
  }
  $menus = menu_get_menus();
  $base_menu_names = variable_get('menu_domain_base_menu_names', array());
  foreach ($base_menu_names as $base_menu_name) {
    $types[$base_menu_name] = array(
      'title' => "{$menus[$base_menu_name]} ($base_menu_name)",
      'category' => t('Menu Domain'),
    );
  }
  return $types;
}

/**
 * Render callback.
 */
function menu_domain_menu_domain_content_type_render($subtype, $conf) {
  $base_menu_name = $subtype;
  $menu_name = _menu_domain_get_menu_name($base_menu_name, $GLOBALS['_domain']);
  if (empty($menu_name)) {
    if ($conf['menu_domain_hide_if_not_detected']) {
      return NULL;
    }
    $menu_name = $base_menu_name;
  }

  switch ($conf['menu_domain_type']) {
    case 'default':
      return (object) array(
        'content' => menu_tree($menu_name),
      );
      break;

    case 'superfish':
      $GLOBALS['conf']['superfish_menu_' . $conf['menu_domain_superfish']] = $menu_name . ':0';
      $block = superfish_block_view($conf['menu_domain_superfish']);
      return (object) array(
        'content' => $block['content'],
      );
      break;
  }

  return NULL;
}

/**
 * Config form callback.
 */
function menu_domain_menu_domain_content_type_edit_form($form, &$form_state) {
  $form['menu_domain_type'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#options' => array(
      'default' => t('Default'),
    ),
    '#default_value' => isset($form_state['conf']['menu_domain_type']) ? $form_state['conf']['menu_domain_type'] : 'default',
  );
  if (module_exists('superfish')) {
    $form['menu_domain_type']['#options']['superfish'] = t('Superfish');
    $blocks = superfish_block_info();
    $options = array();
    foreach ($blocks as $id => $block) {
      $options[$id] = $block['info'];
    }
    $form['menu_domain_superfish'] = array(
      '#type' => 'select',
      '#title' => t('Superfish'),
      '#options' => $options,
      '#default_value' => isset($form_state['conf']['menu_domain_superfish']) ? $form_state['conf']['menu_domain_superfish'] : NULL,
      '#states' => array(
        'visible' => array(
          ':input[name="menu_domain_type"]' => array('value' => 'superfish'),
        ),
      ),
    );
  }
  $form['menu_domain_hide_if_not_detected'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide if not detected'),
    '#description' => t('If there is no menu found for the domain. If checked: don\'t show the menu. Otherwise: show the base menu.'),
    '#default_value' => isset($form_state['conf']['menu_domain_hide_if_not_detected']) ? $form_state['conf']['menu_domain_hide_if_not_detected'] : FALSE,
  );
  return $form;
}

/**
 * Config form submit callback.
 */
function menu_domain_menu_domain_content_type_edit_form_submit($form, &$form_state) {
  $_ = array();
  $form_clean = menu_domain_menu_domain_content_type_edit_form(array(), $_);
  foreach (element_children($form_clean) as $key) {
    $form_state['conf'][$key] = $form_state['values'][$key];
  }
}

/**
 * Admin title callback.
 */
function menu_domain_menu_domain_content_type_admin_title($subtype, $conf) {
  $menus = menu_get_menus();
  return t('Menu Domain') . ': ' . $menus[$subtype] . ' (' . $conf['menu_domain_type'] . ')';
}
