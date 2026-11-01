/* global XMLHttpRequest */
/**
 * --------------------------------------------------------------------
 * docmd : the zero-config documentation engine.
 *
 * Client-side string replacement for noStyle pages.
 * Loads JSON translation files from assets/i18n/ and replaces
 * text content of elements with data-i18n attributes.
 *
 * @package     @docmd/ui
 * @website     https://docmd.io
 * @license     MIT
 * @copyright   Copyright (c) 2025-present docmd.io
 *
 * [docmd-source] - Please do not remove this header.
 * --------------------------------------------------------------------
 */

(function () {
  'use strict';

  var config = window.DOCMD_I18N || {};
  var locales = config.locales || [];
  var defaultLocale = config.default || '';
  var root = window.DOCMD_ROOT || './';

  if (!locales.length) return;

  /**
   * Detect active locale from URL path prefix.
   * Default locale has no prefix (renders at /).
   * Non-default locales render at /{id}/.
   */
  function detectLocale() {
    var saved = localStorage.getItem('docmd-locale');
    var pathSegments = window.location.pathname.split('/').filter(Boolean);
    var firstSegment = pathSegments[0] || '';

    // Check if first path segment matches a non-default locale
    var localeIds = locales.map(function (l) { return l.id; });
    if (localeIds.indexOf(firstSegment) !== -1 && firstSegment !== defaultLocale) {
      return firstSegment;
    }

    // If saved preference matches a configured locale, use it (for default-locale pages)
    if (saved && localeIds.indexOf(saved) !== -1) {
      return saved;
    }

    return defaultLocale;
  }

  /**
   * Load a JSON translation file from assets/i18n/{locale}.json
   */
  function loadStrings(localeId, callback) {
    var url = root + 'assets/i18n/' + localeId + '.json';
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, true);
    xhr.onreadystatechange = function () {
      if (xhr.readyState === 4) {
        if (xhr.status === 200) {
          try {
            callback(null, JSON.parse(xhr.responseText));
          } catch (e) {
            callback(e, null);
          }
        } else {
          callback(new Error('Failed to load ' + url), null);
        }
      }
    };
    xhr.send();
  }

  /**
   * Apply translations to the DOM.
   *
   * Supported attributes:
   *   data-i18n="key"              → replaces textContent
   *   data-i18n-html="key"         → replaces innerHTML
   *   data-i18n-[attr]="key"       → replaces a specific attribute
   *     e.g. data-i18n-placeholder="searchPlaceholder"
   *     e.g. data-i18n-aria-label="navLabel"
   *     e.g. data-i18n-title="tooltipText"
   */
  function applyStrings(strings) {
    if (!strings) return;

    // data-i18n → textContent
    var els = document.querySelectorAll('[data-i18n]');
    for (var i = 0; i < els.length; i++) {
      var key = els[i].getAttribute('data-i18n');
      if (key && strings[key] !== undefined) {
        els[i].textContent = strings[key];
      }
    }

    // data-i18n-html → innerHTML (use cautiously with sanitization)
    var htmlEls = document.querySelectorAll('[data-i18n-html]');
    for (var j = 0; j < htmlEls.length; j++) {
      var htmlKey = htmlEls[j].getAttribute('data-i18n-html');
      if (htmlKey && strings[htmlKey] !== undefined) {
        var rawHtml = strings[htmlKey];
        // Basic sanitization: strip script and event handlers
        var sanitized = rawHtml
          .replace(/<script\b[^>]*>([\s\S]*?)<\/script>/gim, "")
          .replace(/on\w+="[^"]*"/gim, "")
          .replace(/on\w+='[^']*'/gim, "");
        htmlEls[j].innerHTML = sanitized;
      }
    }

    // data-i18n-* → attribute replacement
    // Scan all elements for data-i18n-{attrName} patterns
    var allI18nEls = document.querySelectorAll('*');
    for (var k = 0; k < allI18nEls.length; k++) {
      var el = allI18nEls[k];
      var attrs = el.attributes;
      for (var a = 0; a < attrs.length; a++) {
        var attrName = attrs[a].name;
        if (attrName.indexOf('data-i18n-') === 0 && attrName !== 'data-i18n-html') {
          var targetAttr = attrName.substring('data-i18n-'.length);
          var attrKey = attrs[a].value;
          if (attrKey && strings[attrKey] !== undefined) {
            el.setAttribute(targetAttr, strings[attrKey]);
          }
        }
      }
    }

    // Update html lang attribute
    document.documentElement.setAttribute('lang', activeLocale);

    // Dispatch event for custom handlers
    document.dispatchEvent(new CustomEvent('docmd:i18n-applied', {
      detail: { locale: activeLocale, strings: strings }
    }));
  }

  /**
   * Switch locale - updates localStorage, reloads strings.
   * For multi-page i18n sites, redirects to the locale-prefixed URL.
   * For single-page noStyle sites, swaps strings in place.
   *
   * Set window.DOCMD_I18N.inPlace = true to force in-place mode
   * (no URL redirect, just reload strings on the same page).
   */
  function switchLocale(newLocaleId) {
    if (newLocaleId === activeLocale) return;
    localStorage.setItem('docmd-locale', newLocaleId);

    var inPlace = config.inPlace || false;

    if (inPlace) {
      // In-place mode: just reload strings without navigating
      activeLocale = newLocaleId;
      window.DOCMD_I18N_STRINGS.locale = newLocaleId;
      loadStrings(newLocaleId, function (err, strings) {
        if (!err && strings) applyStrings(strings);
      });
      return;
    }

    var currentPath = window.location.pathname;
    var localeIds = locales.map(function (l) { return l.id; });

    // Strip current locale prefix if present
    var pathSegments = currentPath.split('/').filter(Boolean);
    var basePath = currentPath;
    if (pathSegments.length > 0 && localeIds.indexOf(pathSegments[0]) !== -1 && pathSegments[0] !== defaultLocale) {
      basePath = '/' + pathSegments.slice(1).join('/');
    }

    // Add new locale prefix (skip for default locale)
    var newPath;
    if (newLocaleId === defaultLocale) {
      newPath = basePath || '/';
    } else {
      newPath = '/' + newLocaleId + (basePath === '/' ? '/' : basePath);
    }

    if (newPath !== currentPath) {
      window.location.href = newPath;
    } else {
      // Same page, just reload strings
      activeLocale = newLocaleId;
      window.DOCMD_I18N_STRINGS.locale = newLocaleId;
      loadStrings(newLocaleId, function (err, strings) {
        if (!err && strings) applyStrings(strings);
      });
    }
  }

  // Expose API
  var activeLocale = detectLocale();

  window.DOCMD_I18N_STRINGS = {
    locale: activeLocale,
    locales: locales,
    defaultLocale: defaultLocale,
    switchLocale: switchLocale,
    applyStrings: applyStrings
  };

  function hasI18nElements() {
    if (document.querySelector('[data-i18n], [data-i18n-html]')) return true;
    var allEls = document.querySelectorAll('*');
    for (var i = 0; i < allEls.length; i++) {
      var attrs = allEls[i].attributes;
      for (var j = 0; j < attrs.length; j++) {
        if (attrs[j].name.indexOf('data-i18n-') === 0) return true;
      }
    }
    return false;
  }

  function init() {
    if (hasI18nElements()) {
      loadStrings(activeLocale, function (err, strings) {
        if (!err && strings) applyStrings(strings);
      });
    }
  }

  // Auto-apply on DOM ready
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }
})();