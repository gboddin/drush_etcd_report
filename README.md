# Drupal ETCD Report module

This module allows a Drupal installation to report various details to an etcd 
server through a drush command.

It also supports multisite installations.

## Install

```
git clone https://github.com/gboddin/drush_etcd_report ~/.drush/etcd_report
```

## Example

### Standalone site

```
drush etcd-sync http://etcd-server:2379 drupal/site-name
```

Will populate the following keys on etcd :

  - `drupal/site-name/modules`  List of all modules as json
  - `drupal/site-name/feature-diff` Feature diff
  
### Multisite

```
drush @sites -y etcd-sync http://etcd-server:2379 drupal/@site
```

Will populate all sites in etcd :

  - `drupal/@site/modules` -> List of all modules as json
  - `drupal/@site/feature-diff` -> Feature diff
  
> @site is always replaced by the site alias (calculated from conf_path())

### Get a report

```
$ drush etcdr http://etcd-server:2379 drupal weight      
-----------------weight-----------------
7.x-2.4 installed on site1 site2 site3
7.x-2.5 installed on site4
7.x-2.3 installed on site5 site6 site7 site8
7.x-3.1 installed on site9 site10 site11 site12
```