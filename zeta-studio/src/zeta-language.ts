import type { languages } from 'monaco-editor'

export const zetaLanguage: languages.LanguageConfiguration = {
  comments: {
    lineComment: '//',
    blockComment: ['/*', '*/'],
  },
  brackets: [
    ['(', ')'],
    ['[', ']'],
    ['{', '}'],
    ['<', '>'],
  ],
  autoClosingPairs: [
    { open: '(', close: ')' },
    { open: '[', close: ']' },
    { open: '{', close: '}' },
    { open: '"', close: '"' },
    { open: "'", close: "'" },
  ],
  surroundingPairs: [
    { open: '(', close: ')' },
    { open: '[', close: ']' },
    { open: '{', close: '}' },
    { open: '"', close: '"' },
    { open: "'", close: "'" },
  ],
  folding: {
    markers: {
      start: /^\s*\{/,
      end: /^\s*\}/,
    },
  },
  wordPattern: /(-?\d*\.\d\w*)|([^\`\~\!\@\#\%\^\&\*\(\)\-\=\+\[\{\]\}\\\|\;\:\'\"\,\.\<\>\/\?\s]+)/g,
}

export const zetaMonarch: languages.IMonarchLanguage = {
  ignoreCase: true,
  defaultToken: 'source',
  
  keywords: [
    'fn', 'if', 'else', 'for', 'while', 'in', 'return', 'print',
    'break', 'continue', 'include', 'as', 'export', 'class', 'new',
    'this', 'extends', 'load_csv', 'plot', 'serve', 'metric',
    'dashboard', 'route', 'is_null', 'is_error',
  ],

  builtinFunctions: [
    'mean', 'count', 'sum', 'min', 'max', 'stddev',
    'abs', 'round', 'floor', 'ceil', 'pow', 'sqrt',
    'len', 'upper', 'lower', 'substr', 'reverse', 'sort', 'unique', 'push',
    'keys', 'values', 'type', 'range', 'transpose',
    'head', 'select', 'split', 'join', 'replace', 'find',
    'map', 'filter', 'reduce',
    'group_by', 'agg', 'merge', 'info', 'describe', 'tail', 'sample',
    'value_counts', 'nunique', 'median', 'percentile', 'mode',
    'cor', 'cov', 'isna', 'duplicated', 'cut', 'qcut',
    'dnorm', 'pnorm', 'qnorm', 'dgamma', 'dbeta', 'dunif',
    'dt', 'df', 'dchisq', 't_test', 'anova', 'chi_square', 'linear_regression',
    'cumsum', 'cummax', 'cummin', 'rolling_mean', 'rolling_std',
    'rolling_sum', 'rolling_min', 'rolling_max', 'lag', 'lead',
    'diff', 'row_number', 'rank', 'pct_change',
    'drop_duplicates', 'rename', 'select_cols', 'drop_cols',
    'fillna', 'replace_val', 'clip', 'trim', 'normalize', 'standardize',
    'clear_arena', 'arena_bytes', 'zeta_version',
  ],

  typeKeywords: [
    'NUM', 'STR', 'BOOL', 'VEC', 'MAT', 'DICT', 'DATAFRAME',
    'NULL', 'ERROR', 'FUNCTION', 'SCENE', 'IMAGE', 'TABLE',
    'PLOT', 'HEATMAP', 'DASHBOARD',
  ],

  literals: ['true', 'false', 'null', 'PI', 'E', 'INFINITY'],

  operators: [
    '=', '>', '<', '!', '~', '?', ':',
    '==', '<=', '>=', '!=', '&&', '||',
    '++', '--', '+', '-', '*', '/', '%',
    '<<', '>>', '::', ':?',
  ],

  symbols: /[=><!~?:&|+\-*\/\^%]+/,
  digits: /\d+(_+\d+)*/,

  tokenizer: {
    root: [
      [/\s+/, 'white'],
      [/\/\/.*$/, 'comment'],
      [/\/\*/, 'comment', '@comment'],
      [/"([^"\\]|\\.)*$/, 'string.invalid'],
      [/'([^'\\]|\\.)*$/, 'string.invalid'],
      [/"/, 'string', '@doubleString'],
      [/'/, 'string', '@singleString'],
      [/[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?/, 'number'],
      [/\.[0-9]+([eE][+-]?[0-9]+)?/, 'number'],
      [/\$[a-zA-Z_][a-zA-Z0-9_]*/, 'variable'],
      [/[a-zA-Z_][a-zA-Z0-9_]*/, {
        cases: {
          '@keywords': 'keyword',
          '@builtinFunctions': 'predefined',
          '@typeKeywords': 'type',
          '@literals': 'literal',
          '@default': 'identifier',
        },
      }],
      [/@symbols/, {
        cases: {
          '@operators': 'operator',
          '@default': '',
        },
      }],
      [/[{}()\[\]]/, '@brackets'],
      [/<<>>|<>/, 'delimiter'],
      [/[;,.]/, 'delimiter'],
    ],
    
    comment: [
      [/[^/*]+/, 'comment'],
      [/\*\//, 'comment', '@pop'],
      [/[/*]/, 'comment'],
    ],
    
    doubleString: [
      [/[^\\"]+/, 'string'],
      [/\\./, 'string.escape'],
      [/"/, 'string', '@pop'],
    ],
    
    singleString: [
      [/[^\\']+/, 'string'],
      [/\\./, 'string.escape'],
      [/'/, 'string', '@pop'],
    ],
  },
}
