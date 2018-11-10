<?php
/**
 * Implements hook_drush_command().
 */
function etcd_report_drush_command()
{

    $commands['etcd-report'] = [
        'description' => 'Reports back to ETCD server for inventory, including modules, librairies, content-types.',
        'aliases' => ['etcdr'],
        'arguments' => [
            'arg1' => 'ETCD server address',
            'arg2' => 'ETCD directory (@site can be used in multi-site for site name)',
        ],
        'options' => [
            'user' => 'HTTP basic auth username',
            'pass' => 'HTTP basic auth password',
        ],
        'examples' => [
            'drush etcdr' => 'Print my example command.',
            'drush etcdr https://etc-server:2379 d7/site-name' => 'Reports to etc-server:2379 in the d7/site-name directory.',
        ],
    ];
    return $commands;
}

/**
 * Drush command logic.
 */
function drush_etcd_report($server = "http://localhost:2379", $directory = "d7/@site")
{
    // Get site name from conf_path
    $site_name = str_replace('sites/', '', conf_path());
    $directory = str_replace('@site', $site_name, $directory);

    $user = drush_get_option('user', null);
    $pass = drush_get_option('pass', null);

    drush_print('Getting modules list ...');
    $modules = drush_invoke_process('@self', 'pm-list', array('--format=json'), array(), FALSE);
    _etc_report_send_data($server, $user, $pass, $directory, 'modules', $modules['output']);

    drush_print('Getting features diff ...');
    $featurediff = drush_invoke_process('@self', 'df', array(), array(), FALSE);
    _etc_report_send_data($server, $user, $pass, $directory, 'features-diff', $featurediff['output']);
}

function _etc_report_send_data($server, $user, $pass, $directory, $type, $data)
{
    $result = drupal_http_request(
        $server . '/v2/keys/' . $directory . '/modules',
        [
            "method" => "PUT",
            "headers" => [
                "Authorization" => 'Basic ' . base64_encode($user . ':' . $pass)
            ],
            "data" => "value=" . urlencode($data)
        ]
    );
    if ($result->code != 200) {
        drush_print('Failed to report ' . $type . ' to ' . $server . ' in ' . $directory . '/' . $type . ' ...');
        return false;
    } else {
        drush_print('Reported ' . $type . ' to ' . $server . ' in ' . $directory . '/' . $type . ' ...');
        return true;
    }
}
