#!/usr/bin/env python3
"""
build_site.py - Generates Zeta documentation site.
Outputs: docs/index.html (landing) + docs/docs.html (single-page docs)
"""
import re
import os
import html

VERSION = "0.1.1"
REPO_URL = "https://github.com/juan1417/zeta"
DOCS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROOT_DIR = os.path.dirname(DOCS_DIR)

CHAPTERS = [
    ("01-filosofia.md", "Filosofia y diseno"),
    ("02-sintaxis.md", "Sintaxis"),
    ("03-tipos-y-estructuras.md", "Tipos de datos y estructuras"),
    ("04-control-y-funciones.md", "Control de flujo y funciones"),
    ("05-funciones-nativas.md", "Funciones nativas"),
    ("06-imports-modulos.md", "Imports y modulos"),
    ("07-io.md", "I/O: CSV y HTTP"),
    ("08-errores-null.md", "Errores y null"),
    ("09-escenas-visualizacion.md", "Escenas y visualizacion"),
    ("10-c-abi-loadlib.md", "C ABI / load_lib"),
    ("11-servidor-http.md", "Servidor HTTP"),
    ("12-renderers.md", "Renderers"),
    ("13-arquitectura.md", "Arquitectura interna"),
    ("14-build-paquete.md", "Build y empaquetado"),
    ("15-ejemplos.md", "Ejemplos (cookbook)"),
]

SIDEBAR_CATEGORIES = [
    ("Introduccion", [0, 1, 2, 3]),
    ("Referencia", [4, 5, 6, 7]),
    ("Visualizacion", [8, 9, 10, 11]),
    ("Interno", [12, 13, 14]),
]


def esc(s):
    return html.escape(s)


def md_to_html(md):
    lines = md.split('\n')
    result = []
    i = 0
    in_code = False
    code_lang = ''
    code_lines = []
    in_table = False
    table_rows = []

    while i < len(lines):
        line = lines[i]

        if line.strip().startswith('```'):
            if in_code:
                code_content = esc('\n'.join(code_lines))
                lang_class = f' class="language-{code_lang}"' if code_lang else ''
                result.append(f'<pre><code{lang_class}>{code_content}</code></pre>')
                code_lines = []
                in_code = False
            else:
                in_code = True
                code_lang = line.strip()[3:].strip()
            i += 1
            continue

        if in_code:
            code_lines.append(line)
            i += 1
            continue

        if in_table and not line.strip().startswith('|'):
            result.append(render_table(table_rows))
            table_rows = []
            in_table = False

        stripped = line.strip()
        if not stripped:
            i += 1
            continue

        if re.match(r'^---+$', stripped):
            result.append('<hr>')
            i += 1
            continue

        hm = re.match(r'^(#{1,6})\s+(.+)', stripped)
        if hm:
            level = len(hm.group(1))
            text = inline_fmt(hm.group(2))
            anchor = re.sub(r'[^a-z0-9]+', '-', hm.group(2).lower()).strip('-')
            result.append(f'<h{level} id="{anchor}">{text}</h{level}>')
            i += 1
            continue

        if stripped.startswith('|') and stripped.endswith('|'):
            if re.match(r'^\|[\s\-:|]+\|$', stripped):
                i += 1
                continue
            cells = [c.strip() for c in stripped.split('|')[1:-1]]
            table_rows.append(cells)
            in_table = True
            i += 1
            continue

        if stripped.startswith('>'):
            bq_lines = []
            while i < len(lines) and lines[i].strip().startswith('>'):
                bq_lines.append(re.sub(r'^>\s?', '', lines[i].strip()))
                i += 1
            result.append(f'<blockquote><p>{inline_fmt(" ".join(bq_lines))}</p></blockquote>')
            continue

        if re.match(r'^[-*+]\s', stripped):
            result.append('<ul>')
            while i < len(lines):
                s = lines[i].strip()
                if re.match(r'^[-*+]\s', s):
                    result.append(f'<li>{inline_fmt(re.sub(r"^[-*+]\s+", "", s))}</li>')
                    i += 1
                elif not s:
                    j = i + 1
                    while j < len(lines) and not lines[j].strip():
                        j += 1
                    if j < len(lines) and re.match(r'^[-*+]\s', lines[j].strip()):
                        i = j
                    else:
                        break
                else:
                    break
            result.append('</ul>')
            continue

        if re.match(r'^\d+\.\s', stripped):
            result.append('<ol>')
            while i < len(lines):
                s = lines[i].strip()
                if re.match(r'^\d+\.\s', s):
                    result.append(f'<li>{inline_fmt(re.sub(r"^\d+\.\s+", "", s))}</li>')
                    i += 1
                elif not s:
                    j = i + 1
                    while j < len(lines) and not lines[j].strip():
                        j += 1
                    if j < len(lines) and re.match(r'^\d+\.\s', lines[j].strip()):
                        i = j
                    else:
                        break
                else:
                    break
            result.append('</ol>')
            continue

        para_lines = []
        while i < len(lines) and lines[i].strip() and not lines[i].strip().startswith('#') and not lines[i].strip().startswith('```') and not lines[i].strip().startswith('|') and not lines[i].strip().startswith('>') and not re.match(r'^[-*+]\s', lines[i].strip()) and not re.match(r'^\d+\.\s', lines[i].strip()) and not re.match(r'^---+$', lines[i].strip()):
            para_lines.append(lines[i].strip())
            i += 1
        if para_lines:
            result.append(f'<p>{inline_fmt(" ".join(para_lines))}</p>')
            continue

        i += 1

    if in_table and table_rows:
        result.append(render_table(table_rows))

    return '\n'.join(result)


def inline_fmt(text):
    text = re.sub(r'`([^`]+)`', r'<code>\1</code>', text)
    text = re.sub(r'\*\*\*(.+?)\*\*\*', r'<strong><em>\1</em></strong>', text)
    text = re.sub(r'\*\*(.+?)\*\*', r'<strong>\1</strong>', text)
    text = re.sub(r'\*(.+?)\*', r'<em>\1</em>', text)
    text = re.sub(r'\[([^\]]+)\]\(([^)]+)\)', r'<a href="\2">\1</a>', text)
    return text


def render_table(rows):
    if not rows:
        return ''
    h = ['<table>', '<thead><tr>']
    for c in rows[0]:
        h.append(f'<th>{inline_fmt(c)}</th>')
    h.append('</tr></thead>')
    if len(rows) > 1:
        h.append('<tbody>')
        for row in rows[1:]:
            h.append('<tr>')
            for c in row:
                h.append(f'<td>{inline_fmt(c)}</td>')
            h.append('</tr>')
        h.append('</tbody>')
    h.append('</table>')
    return '\n'.join(h)


def read_md(filename):
    path = os.path.join(DOCS_DIR, filename)
    if not os.path.exists(path):
        path = os.path.join(ROOT_DIR, filename)
    with open(path, 'r', encoding='utf-8') as f:
        return f.read()


def build_docs_html():
    sidebar_parts = []
    for cat_name, chapter_indices in SIDEBAR_CATEGORIES:
        sidebar_parts.append(f'<div class="sidebar-category">')
        sidebar_parts.append(f'  <div class="sidebar-category-title">{esc(cat_name)}</div>')
        for idx in chapter_indices:
            fname, title = CHAPTERS[idx]
            num = f"{idx+1:02d}"
            ch_id = fname.replace(".md", "")
            sidebar_parts.append(f'  <a href="#ch{num}" class="sidebar-link" data-ch="ch{num}"><span class="num">{num}</span> {esc(title)}</a>')
        sidebar_parts.append('</div>')
    sidebar_html = '\n'.join(sidebar_parts)

    toc_parts = ['<div class="toc-title" data-i18n="toc">En esta pagina</div>']
    for idx, (fname, title) in enumerate(CHAPTERS):
        num = f"{idx+1:02d}"
        short = title.split(':')[0].strip() if ':' in title else title.split(' - ')[0].strip() if ' - ' in title else title
        toc_parts.append(f'<a href="#ch{num}">{num} — {esc(short)}</a>')
    toc_html = '\n'.join(toc_parts)

    content_parts = []
    for idx, (fname, title) in enumerate(CHAPTERS):
        num = f"{idx+1:02d}"
        md = read_md(fname)
        content_html = md_to_html(md)
        content_parts.append(f'<section id="ch{num}">')
        content_parts.append(content_html)
        content_parts.append('</section>')
        if idx < len(CHAPTERS) - 1:
            content_parts.append('<hr>')
    content_html = '\n'.join(content_parts)

    nav_prev = '<a href="index.html" data-i18n="nav.back_home">&larr; Volver al inicio</a>'
    nav_next = f'<a href="{REPO_URL}" target="_blank" data-i18n="nav.view_github">Ver en GitHub &rarr;</a>'

    return f'''<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Documentacion — Zeta</title>
  <link rel="icon" type="image/x-icon" href="favicon.ico">
  <link rel="icon" type="image/png" sizes="32x32" href="favicon-32x32.png">
  <link rel="icon" type="image/png" sizes="16x16" href="favicon-16x16.png">
  <link rel="apple-touch-icon" sizes="180x180" href="apple-touch-icon.png">
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&family=JetBrains+Mono:wght@400;500&display=swap" rel="stylesheet">
  <link rel="stylesheet" href="css/style.css">
</head>
<body>

  <nav class="navbar">
    <div class="navbar-inner">
      <a href="index.html" class="navbar-brand">
        <img src="logo.png" alt="Zeta" class="navbar-logo">
        Zeta
      </a>
      <ul class="nav-links" id="navLinks">
        <li><a href="index.html" data-i18n="nav.home">Inicio</a></li>
        <li><a href="index.html#features" data-i18n="nav.features">Features</a></li>
        <li><a href="index.html#download" data-i18n="nav.install">Download</a></li>
        <li><a href="docs.html" class="active" data-i18n="nav.docs">Docs</a></li>
        <li>
          <a href="{REPO_URL}" target="_blank" rel="noopener" class="btn-repo" data-i18n="nav.repo">
            <svg viewBox="0 0 16 16" aria-hidden="true"><path d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0016 8c0-4.42-3.58-8-8-8z"/></svg>
            Repositorio
          </a>
        </li>
        <li><button id="lang-toggle" class="lang-toggle" title="Switch to English"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"/><path d="M2 12h20"/><path d="M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10z"/></svg><span id="lang-label">EN</span></button></li>
      </ul>
      <button class="navbar-toggle" id="navToggle" aria-label="Menu">
        <svg viewBox="0 0 24 24"><path d="M3 6h18v2H3V6zm0 5h18v2H3v-2zm0 5h18v2H3v-2z"/></svg>
      </button>
    </div>
  </nav>

  <div class="docs-layout">
    <aside class="sidebar" id="sidebar">
      {sidebar_html}
    </aside>

    <main class="docs-main" id="docsContent">
      {content_html}

      <div class="chapter-nav">
        {nav_prev}
        {nav_next}
      </div>
    </main>

    <nav class="toc" id="toc">
      {toc_html}
    </nav>
  </div>

  <button class="mobile-menu-btn" id="mobileMenuBtn" aria-label="Menu">&#9776;</button>

  <script src="js/i18n.js"></script>
  <script src="js/main.js"></script>
</body>
</html>'''


if __name__ == '__main__':
    count = 0

    # Docs page (single-page)
    out = os.path.join(DOCS_DIR, 'docs.html')
    with open(out, 'w', encoding='utf-8') as f:
        f.write(build_docs_html())
    sz = os.path.getsize(out)
    print(f"[{count+1}] {out} ({sz:,} bytes)")
    count += 1

    print(f"\nDone: {count} page generated")
    print("Note: docs/index.html is now hand-written (mockup design).")
    print("Run: python3 docs/scripts/build_site.py to regenerate docs.html only.")
