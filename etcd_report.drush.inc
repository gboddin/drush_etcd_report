<?php
/**
 * Implements hook_drush_command().
 */
function etcd_report_drush_command()
{

    $commands['etcd-sync'] = [
        'description' => 'Reports back to ETCD server for inventory, including modules, librairies, content-types.',
        'aliases' => ['etcds'],
//        'bootstrap' => DRUSH_BOOTSTRAP_NONE,
        'arguments' => [
            'arg1' => 'ETCD server address',
            'arg2' => 'ETCD directory (@site can be used in multi-site for site name)',
        ],
        'options' => [
            'user' => 'HTTP basic auth username',
            'pass' => 'HTTP basic auth password',
        ],
        'examples' => [
            'drush etcds https://etc-server:2379 d7/site-name' => 'Reports to etc-server:2379 in the d7/site-name directory.',
            'drush @sites -y etcds https://etc-server:2379 d7/@site' => 'Reports multisite to etc-server:2379 in the d7/site-name directory.',
        ],
    ];
    $commands['etcd-module-report'] = [
        'description' => 'Get a module report from ETCD.',
        'aliases' => ['etcdr'],
#        'bootstrap' => DRUSH_BOOTSTRAP_NONE,
        'arguments' => [
            'arg1' => 'ETCD server address',
            'arg2' => 'ETCD directory containing sites',
            'arg3' => 'Drupal module name',
        ],
        'options' => [
            'user' => 'HTTP basic auth username',
            'pass' => 'HTTP basic auth password',
        ],
        'examples' => [
            'drush etcdm https://etc-server:2379 d7 media' => 'Generates a module report for all sites in the d7 directory.',
        ],
    ];

    return $commands;
}

/**
 * Drush command logic.
 */
function drush_etcd_report_etcd_sync($server = "http://localhost:2379", $directory = "d7/@site")
{
    // Get site name from conf_path
    $site_name = str_replace('sites/', '', conf_path());
    $directory = str_replace('@site', $site_name, $directory);

    $user = drush_get_option('user', null);
    $pass = drush_get_option('pass', null);

    drush_print('Getting modules list ...');
    $modules = drush_invoke_process('@self', 'pm-list', array('--format=json', '--no-core'), array(), FALSE);
    _etc_report_send_data($server, $user, $pass, $directory, 'modules', $modules['output']);

    drush_print('Getting features diff ...');
    $featurediff = drush_invoke_process('@self', 'df', array(), array(), FALSE);
    _etc_report_send_data($server, $user, $pass, $directory, 'features-diff', $featurediff['output']);
}

function drush_etcd_report_etcd_module_report($server = "http://localhost:2379", $directory = "d7", $module_arg = null)
{
    $user = drush_get_option('user', null);
    $pass = drush_get_option('pass', null);
    $sites = [];
    # Build useful data arrays :
    foreach (_etc_report_get_data($server, $user, $pass, $directory, 'modules')->node->nodes as $site) {
        foreach ($site->nodes as $site_details) {
            if ($site_details->key == $site->key . '/modules') {
                if (empty($site_details->value)) continue;
                $sites[$site->key] = json_decode($site_details->value, true);
                foreach ($sites[$site->key] as $module_name => $module) {
                    $modules[$module_name][$module['version']][] = str_replace('/' . $directory . '/', '', $site->key);
                }
            }
        }
    }

    # Print results
    foreach ($modules as $module_name => $module_version) {
        if (!empty($module_arg) && $module_name != $module_arg) continue;
        drush_print(str_pad($module_name, 40, "-", STR_PAD_BOTH));
        foreach ($module_version as $version => $sites) {
            echo $version . ' installed on ' . implode(' ', $sites) . PHP_EOL;
        }
    }
}

function _etc_report_get_data($server, $user, $pass, $directory, $type)
{
    $result = drupal_http_request(
        $server . '/v2/keys/' . $directory . '?recursive=true',
        [
            "method" => "GET",
            "headers" => [
                "Authorization" => 'Basic ' . base64_encode($user . ':' . $pass)
            ]
        ]
    );
    if ($result->code == 200) {
        return json_decode($result->data);
    }
    return false;
}

function _etc_report_send_data($server, $user, $pass, $directory, $type, $data)
{
    $result = drupal_http_request(
        $server . '/v2/keys/' . $directory . '/' . $type,
        [
            "method" => "PUT",
            "headers" => [
                "Authorization" => 'Basic ' . base64_encode($user . ':' . $pass)
            ],
            "data" => "value=" . urlencode($data)
        ]
    );
    if ($result->code == 200 || $result->code == 201) {
        drush_print('Reported ' . $type . ' to ' . $server . ' in ' . $directory . '/' . $type . ' ...');
        return true;
    } else {
        drush_print('Failed to report ' . $type . ' to ' . $server . ' in ' . $directory . '/' . $type . ' ...');
        return false;
    }
}
