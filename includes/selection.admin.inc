<?php

/**
 * Admin settings.
 */
function selection_overview() {
  $sql = "SELECT * FROM {selections} ORDER BY sid ASC";
  $result = db_query($sql);
  $rows = array();
  while ($row = db_fetch_object($result)) {
    $rows[] = array(
      $row->name,
      l($row->path, $row->path),
      l(t('Edit'), 'admin/build/selection/'. $row->sid .'/edit'),
      l(t('Delete'), 'admin/build/selection/'. $row->sid .'/delete'),
    );
  }
  $header = array( 
    t('Name'), 
    t('Path'), 
    array(
      'data' => t('Operations'),
      'colspan' => 2,
    ),
  );
  if (empty($rows)) {
    $rows[] = array('data' => t('Not found. Please add new one.'), 'colspan' => 4);
  }
  $output = theme('table', $header, $rows);
  return $output;
}

/** 
 * Add form.
 */
function selection_add_form() {
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Selection name'),
  );
  $form['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path'),
  );
  $types = array();
  foreach (node_get_types() as $type) {
    $types[$type->type] = $type->name;
  }
  $form['node_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Content types'),
    '#options' => $types,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  return $form;
}

/** 
 * Add form callback.
 */
function selection_add_form_submit(&$form, $form_state) {
  $values = $form_state['values'];
  db_query("INSERT INTO {selections} (name, path) VALUES ('%s', '%s')", $values['name'], $values['path']);
  $sid = db_last_insert_id('selections', 'sid');
  foreach ($values['node_types'] as $type) {
    if (!empty($type)) {
      db_query("INSERT INTO {selections_filter} (sid, type, value) VALUES (%d, '%s', '%s')", $sid, 'node', $type);
    }
  }
  $form['#redirect'] = 'admin/build/selection';
}

/**
 * Edit form.
 */
function selection_edit_form($form_state, $selection) {
  $form = selection_add_form();
  $form['name']['#default_value'] = $selection->name;
  $form['path']['#default_value'] = $selection->path;
  $types = array();
  $sql = "SELECT value FROM {selections_filter} WHERE sid = %d";
  $result = db_query($sql, $selection->sid);
  while ($row = db_fetch_object($result)) {
    $types[$row->value] = $row->value;
  }
  $form['node_types']['#default_value'] = $types;
  $form['sid'] = array(
    '#type' => 'hidden',
    '#value' => $selection->sid,
  );
  return $form;
}

/**
 * Edit form submit.
 */
function selection_edit_form_submit(&$form, $form_state) {
  $values = $form_state['values'];
  db_query("UPDATE {selections} SET name = '%s', path = '%s' WHERE sid = %d", $values['name'], $values['path'], $values['sid']);
  db_query("DELETE FROM {selections_filter} WHERE sid = %d", $values['sid']);
  foreach ($values['node_types'] as $type) {
    if (!empty($type)) {
      db_query("INSERT INTO {selections_filter} (sid, type, value) VALUES (%d, '%s', '%s')", $values['sid'], 'node', $type);
    }
  }
  drupal_set_message(t('Selection @name has been saved.', array('@name' => $name)));
  $form['#redirect'] = 'admin/build/selection';
}

/**
 * Delete form.
 */
function selection_delete_form($form_state, $selection) {
  $form['markup'] = array(
    '#type' => 'markup',
    '#value' => '<div>'. t('This action cannot be undone.') .'</div>',
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#suffix' => l(t('Cancel'), 'admin/build/selection'),
  );
  $form['sid'] = array(
    '#type' => 'hidden',
    '#value' => $selection->sid,
  );
  $form['name'] = array(
    '#type' => 'hidden',
    '#value' => $selection->name,
  );
  return $form;
}

/**
 * Delete form submit.
 */
function selection_delete_form_submit(&$form, $form_state) {
  $sid = $form_state['values']['sid'];
  $name = $form_state['values']['name'];
  db_query("DELETE FROM {selections} WHERE sid = %d", $sid);
  db_query("DELETE FROM {selections_filter} WHERE sid = %d", $sid);
  drupal_set_message(t('Selection @name has been deleted.', array('@name' => $name)));
  $form['#redirect'] = 'admin/build/selection';
}

/**
 * Clear caches.
 */
function selection_clear() {
  cache_clear_all();
  drupal_goto('admin/build/selection');
}
