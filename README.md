# Drupal ETCD Report module

## Install

```bash
git clone https://github.com/gboddin/drush_etcd_report ~/.drush/etcd_report
```

## Example

### Standalone site

```bash
drush etcdr http://etcd-server:2379 drupal/site-name
```

Will populate the following keys on etcd :

  - drupal/site-name/modules -> List of all modules as json
  - drupal/site-name/feature-diff -> Feature diff
  
### Multisite

```bash
drush @sites -y etcdr http://etcd-server:2379 drupal/@site
```

Will populate all sites in etcd :

  - drupal/@site/modules -> List of all modules as json
  - drupal/@site/feature-diff -> Feature diff
  
> @site is always replaced by the site alias (calculated from conf_path())

