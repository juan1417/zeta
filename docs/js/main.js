(function() {
  'use strict';

  // Mobile nav toggle
  var toggle = document.getElementById('navToggle');
  var links = document.getElementById('navLinks');
  if (toggle && links) {
    toggle.addEventListener('click', function() {
      links.classList.toggle('open');
    });
    links.querySelectorAll('a').forEach(function(link) {
      link.addEventListener('click', function() {
        links.classList.remove('open');
      });
    });
  }

  // Install tabs
  window.switchInstallTab = function(os) {
    document.querySelectorAll('.install-tab').forEach(function(tab) {
      tab.classList.toggle('active', tab.dataset.os === os);
    });
    document.querySelectorAll('.install-panel').forEach(function(panel) {
      panel.classList.toggle('active', panel.id === 'install-' + os);
    });
  };

  // Detect OS and set primary download button
  var ua = navigator.userAgent.toLowerCase();
  var os = ua.includes('mac') ? 'macos' : ua.includes('win') ? 'windows' : 'linux';
  var btnMain = document.getElementById('btn-download-main');
  var labelMain = document.getElementById('download-main-label');
  var fileMain = document.getElementById('download-main-file');

  var osData = {
    linux:  { label: 'Descargar para Linux',  file: 'zeta-linux-x64.tar.gz',  href: 'https://github.com/juan1417/zeta/releases/latest/download/zeta-linux-x64.tar.gz' },
    macos:  { label: 'Descargar para macOS',  file: 'zeta-macos-arm64.tar.gz', href: 'https://github.com/juan1417/zeta/releases/latest/download/zeta-macos-arm64.tar.gz' },
    windows:{ label: 'Descargar para Windows', file: 'zeta-windows-x64.zip',   href: 'https://github.com/juan1417/zeta/releases/latest/download/zeta-windows-x64.zip' }
  };

  if (osData[os] && btnMain && labelMain && fileMain) {
    labelMain.textContent = osData[os].label;
    fileMain.textContent = osData[os].file;
    btnMain.href = osData[os].href;
  }

  if (typeof switchInstallTab === 'function') {
    switchInstallTab(os);
  }

  // Scroll reveal
  var revealElements = document.querySelectorAll('.reveal');
  var observer = new IntersectionObserver(function(entries) {
    entries.forEach(function(entry) {
      if (entry.isIntersecting) {
        entry.target.classList.add('visible');
      }
    });
  }, { threshold: 0.1, rootMargin: '0px 0px -40px 0px' });

  revealElements.forEach(function(el) { observer.observe(el); });

  // Active nav link highlight
  var sections = document.querySelectorAll('section[id]');
  var navLinksAll = document.querySelectorAll('.nav-links a[href^="#"]');
  window.addEventListener('scroll', function() {
    var current = '';
    sections.forEach(function(section) {
      var sectionTop = section.offsetTop - 80;
      if (window.scrollY >= sectionTop) {
        current = section.getAttribute('id');
      }
    });
    navLinksAll.forEach(function(link) {
      link.style.color = '';
      if (link.getAttribute('href') === '#' + current) {
        link.style.color = 'var(--fg)';
      }
    });
  });

  // Sidebar mobile toggle (docs page)
  var mobileBtn = document.getElementById('mobileMenuBtn');
  var sidebar = document.getElementById('sidebar');
  if (mobileBtn && sidebar) {
    mobileBtn.addEventListener('click', function() {
      sidebar.classList.toggle('open');
    });
  }

  // Sidebar scroll spy (docs page)
  var sidebarLinks = document.querySelectorAll('.sidebar-link');
  var docSections = [];
  sidebarLinks.forEach(function(link) {
    var href = link.getAttribute('href');
    if (href && href.startsWith('#')) {
      var el = document.getElementById(href.substring(1));
      if (el) docSections.push({ el: el, link: link });
    }
  });

  if (docSections.length > 0) {
    window.addEventListener('scroll', function() {
      var scrollY = window.scrollY + 120;
      var current = null;
      for (var i = docSections.length - 1; i >= 0; i--) {
        if (docSections[i].el.offsetTop <= scrollY) {
          current = docSections[i];
          break;
        }
      }
      sidebarLinks.forEach(function(l) { l.classList.remove('active'); });
      if (current) current.link.classList.add('active');
    });
  }

  // Close sidebar on mobile after click
  sidebarLinks.forEach(function(link) {
    link.addEventListener('click', function() {
      if (window.innerWidth <= 900 && sidebar) {
        sidebar.classList.remove('open');
      }
    });
  });

  // Syntax highlighting
  var ZETA_KEYWORDS = [
    'fn', 'if', 'else', 'for', 'while', 'in', 'return', 'break', 'continue',
    'include', 'as', 'export', 'class', 'new', 'extends',
    'scene', 'layout', 'add_metric', 'add_line_plot', 'add_bar_chart',
    'add_scatter', 'add_histogram', 'add_box_plot', 'add_heatmap',
    'add_linear_regression', 'add_kpi'
  ];

  var ZETA_BUILTINS = [
    'print', 'load_csv', 'load_json', 'load_xlsx', 'save_csv', 'save_xlsx',
    'is_null', 'is_error', 'is_num', 'is_str', 'is_vec', 'is_bool', 'is_df',
    'mk_err', 'mk_null_val',
    'mean', 'count', 'sum', 'min', 'max', 'stddev', 'variance', 'median',
    'abs', 'round', 'floor', 'ceil', 'pow', 'sqrt', 'log', 'exp',
    'len', 'upper', 'lower', 'substr', 'split', 'join', 'replace', 'find',
    'reverse', 'sort', 'unique', 'push', 'pop',
    'keys', 'values', 'type', 'range',
    'transpose', 'dot', 'head', 'select', 'drop', 'drop_nan',
    'map', 'filter', 'reduce', 'format',
    'time', 'serve', 'plot', 'route', 'guardar_grafo', 'cargar_grafo', 'grafo_actual',
    'load_lib'
  ];

  var ZETA_CONSTANTS = ['true', 'false', 'null', 'PI', 'E', 'INFINITY'];

  function escapeHtml(s) {
    return s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
  }

  function tokenizeZeta(code) {
    var result = '';
    var i = 0;
    var len = code.length;
    while (i < len) {
      if (code[i] === '#') {
        var end = code.indexOf('\n', i);
        if (end === -1) end = len;
        result += '<span class="cmt">' + escapeHtml(code.slice(i, end)) + '</span>';
        i = end;
        continue;
      }
      if (code[i] === '"' || code[i] === "'") {
        var q = code[i];
        var j = i + 1;
        while (j < len && code[j] !== q) { if (code[j] === '\\') j++; j++; }
        j = Math.min(j + 1, len);
        result += '<span class="str">' + escapeHtml(code.slice(i, j)) + '</span>';
        i = j;
        continue;
      }
      if (/[0-9]/.test(code[i]) || (code[i] === '.' && i + 1 < len && /[0-9]/.test(code[i + 1]))) {
        var numEnd = i;
        while (numEnd < len && /[0-9]/.test(code[numEnd])) numEnd++;
        while (numEnd < len && code[numEnd] === ',') { numEnd++; while (numEnd < len && /[0-9]/.test(code[numEnd])) numEnd++; }
        if (numEnd < len && code[numEnd] === '.') { numEnd++; while (numEnd < len && /[0-9]/.test(code[numEnd])) numEnd++; }
        if (numEnd < len && (code[numEnd] === 'e' || code[numEnd] === 'E')) { numEnd++; if (numEnd < len && (code[numEnd] === '+' || code[numEnd] === '-')) numEnd++; while (numEnd < len && /[0-9]/.test(code[numEnd])) numEnd++; }
        result += '<span class="num">' + escapeHtml(code.slice(i, numEnd)) + '</span>';
        i = numEnd;
        continue;
      }
      if (/[a-zA-Z_$]/.test(code[i])) {
        var wEnd = i;
        while (wEnd < len && /[a-zA-Z0-9_$]/.test(code[wEnd])) wEnd++;
        var word = code.slice(i, wEnd);
        if (ZETA_KEYWORDS.indexOf(word) !== -1) {
          result += '<span class="kw">' + escapeHtml(word) + '</span>';
        } else if (ZETA_CONSTANTS.indexOf(word) !== -1) {
          result += '<span class="blt">' + escapeHtml(word) + '</span>';
        } else if (ZETA_BUILTINS.indexOf(word) !== -1) {
          result += '<span class="fn">' + escapeHtml(word) + '</span>';
        } else if (wEnd < len && code[wEnd] === '(') {
          result += '<span class="fn">' + escapeHtml(word) + '</span>';
        } else {
          result += escapeHtml(word);
        }
        i = wEnd;
        continue;
      }
      if ('+-*/%=!<>&|?:'.indexOf(code[i]) !== -1) {
        var op = code[i];
        if (i + 1 < len && (code.slice(i, i+2) === '==' || code.slice(i, i+2) === '!=' ||
            code.slice(i, i+2) === '<=' || code.slice(i, i+2) === '>=' ||
            code.slice(i, i+2) === '&&' || code.slice(i, i+2) === '||')) {
          op = code.slice(i, i + 2);
        }
        result += '<span class="op">' + escapeHtml(op) + '</span>';
        i += op.length;
        continue;
      }
      result += escapeHtml(code[i]);
      i++;
    }
    return result;
  }

  function tokenizeBash(code) {
    var result = '';
    var lines = code.split('\n');
    for (var li = 0; li < lines.length; li++) {
      var line = lines[li];
      if (li > 0) result += '\n';
      if (/^\s*#/.test(line)) { result += '<span class="cmt">' + escapeHtml(line) + '</span>'; continue; }
      var parts = line.split(/(\s+)/);
      for (var pi = 0; pi < parts.length; pi++) {
        var p = parts[pi];
        if (/^\s+$/.test(p)) { result += p; continue; }
        if (p === '|' || p === '&&' || p === '||' || p === '>' || p === '>>' || p === '<' || p === '2>' || p === ';' || p === ';;') {
          result += '<span class="op">' + escapeHtml(p) + '</span>';
        } else if (/^(if|then|else|elif|fi|for|do|done|while|case|esac|echo|cd|mkdir|cp|mv|rm|cat|chmod|sudo|apt|pip|clang|make|git|curl|wget|tar|grep|sed|awk|find|ls|pwd|exit|set|export|source|function)$/.test(p)) {
          result += '<span class="kw">' + escapeHtml(p) + '</span>';
        } else if (/^["']/.test(p)) {
          result += '<span class="str">' + escapeHtml(p) + '</span>';
        } else if (/^-[a-zA-Z]+/.test(p)) {
          result += '<span class="op">' + escapeHtml(p) + '</span>';
        } else if (/^\$/.test(p)) {
          result += '<span class="typ">' + escapeHtml(p) + '</span>';
        } else {
          result += escapeHtml(p);
        }
      }
    }
    return result;
  }

  function tokenizeJson(code) {
    var result = '';
    var i = 0;
    var len = code.length;
    while (i < len) {
      if (code[i] === '"') {
        var j = i + 1;
        while (j < len && code[j] !== '"') { if (code[j] === '\\') j++; j++; }
        j = Math.min(j + 1, len);
        result += '<span class="str">' + escapeHtml(code.slice(i, j)) + '</span>';
        i = j;
        continue;
      }
      if (/[0-9\-]/.test(code[i])) {
        var nEnd = i;
        if (code[i] === '-') nEnd++;
        while (nEnd < len && /[0-9]/.test(code[nEnd])) nEnd++;
        if (nEnd < len && code[nEnd] === '.') { nEnd++; while (nEnd < len && /[0-9]/.test(code[nEnd])) nEnd++; }
        if (nEnd < len && (code[nEnd] === 'e' || code[nEnd] === 'E')) { nEnd++; if (nEnd < len && (code[nEnd]==='+'||code[nEnd]==='-')) nEnd++; while (nEnd<len&&/[0-9]/.test(code[nEnd])) nEnd++; }
        result += '<span class="num">' + escapeHtml(code.slice(i, nEnd)) + '</span>';
        i = nEnd;
        continue;
      }
      if (code.slice(i, i+4) === 'true' || code.slice(i, i+5) === 'false' || code.slice(i, i+4) === 'null') {
        var w = code.slice(i, i+4) === 'true' || code.slice(i, i+4) === 'null' ? code.slice(i, i+4) : code.slice(i, i+5);
        result += '<span class="blt">' + escapeHtml(w) + '</span>';
        i += w.length;
        continue;
      }
      result += escapeHtml(code[i]);
      i++;
    }
    return result;
  }

  document.addEventListener('DOMContentLoaded', function() {
    var blocks = document.querySelectorAll('pre code');
    for (var i = 0; i < blocks.length; i++) {
      var el = blocks[i];
      var code = el.textContent;
      var lang = '';
      var cls = el.className || '';
      var m = cls.match(/language-(\w+)/);
      if (m) lang = m[1];

      var highlighted;
      if (lang === 'zeta' || lang === 'zl') {
        highlighted = tokenizeZeta(code);
      } else if (lang === 'bash' || lang === 'sh' || lang === 'shell') {
        highlighted = tokenizeBash(code);
      } else if (lang === 'json') {
        highlighted = tokenizeJson(code);
      } else {
        highlighted = escapeHtml(code);
      }
      el.innerHTML = highlighted;
    }
  });
})();
