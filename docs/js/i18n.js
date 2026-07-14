(function() {
  'use strict';

  var currentLang = localStorage.getItem('zeta-lang') || 'es';
  var translations = {};
  var loaded = {};

  function getNestedVal(obj, path) {
    var parts = path.split('.');
    var current = obj;
    for (var i = 0; i < parts.length; i++) {
      if (current === null || current === undefined) return null;
      current = current[parts[i]];
    }
    return current;
  }

  function applyTranslations() {
    var elements = document.querySelectorAll('[data-i18n]');
    for (var i = 0; i < elements.length; i++) {
      var el = elements[i];
      var key = el.getAttribute('data-i18n');
      var val = getNestedVal(translations, key);
      if (val !== null && val !== undefined) {
        el.innerHTML = val;
      }
    }

    var attrElements = document.querySelectorAll('[data-i18n-attr]');
    for (var j = 0; j < attrElements.length; j++) {
      var el2 = attrElements[j];
      var attrPairs = el2.getAttribute('data-i18n-attr').split(',');
      for (var k = 0; k < attrPairs.length; k++) {
        var pair = attrPairs[k].split(':');
        if (pair.length === 2) {
          var attr = pair[0].trim();
          var key2 = pair[1].trim();
          var val2 = getNestedVal(translations, key2);
          if (val2 !== null && val2 !== undefined) {
            el2.setAttribute(attr, val2);
          }
        }
      }
    }

    document.documentElement.lang = currentLang;

    var toggleBtn = document.getElementById('lang-toggle');
    if (toggleBtn) {
      var langLabel = document.getElementById('lang-label');
      if (langLabel) langLabel.textContent = currentLang === 'es' ? 'EN' : 'ES';
      toggleBtn.title = currentLang === 'es' ? 'Switch to English' : 'Cambiar a español';
    }
  }

  function loadLang(lang, callback) {
    if (loaded[lang]) {
      translations = loaded[lang];
      applyTranslations();
      if (callback) callback();
      return;
    }

    var xhr = new XMLHttpRequest();
    xhr.open('GET', 'i18n/' + lang + '.json', true);
    xhr.onreadystatechange = function() {
      if (xhr.readyState === 4) {
        if (xhr.status === 200) {
          loaded[lang] = JSON.parse(xhr.responseText);
          translations = loaded[lang];
          applyTranslations();
        }
        if (callback) callback();
      }
    };
    xhr.send();
  }

  function setLanguage(lang) {
    currentLang = lang;
    localStorage.setItem('zeta-lang', lang);
    if (lang === 'es') {
      translations = {};
      applyTranslations();
    } else {
      loadLang(lang);
    }
  }

  function toggleLanguage() {
    var next = currentLang === 'es' ? 'en' : 'es';
    setLanguage(next);
  }

  window.ZetaI18n = {
    setLanguage: setLanguage,
    toggleLanguage: toggleLanguage,
    getLanguage: function() { return currentLang; }
  };

  document.addEventListener('DOMContentLoaded', function() {
    var toggleBtn = document.getElementById('lang-toggle');
    if (toggleBtn) {
      toggleBtn.addEventListener('click', toggleLanguage);
    }

    if (currentLang !== 'es') {
      loadLang(currentLang);
    } else {
      applyTranslations();
    }
  });
})();
