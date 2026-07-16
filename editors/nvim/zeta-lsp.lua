-- Zeta Language Support for Neovim
-- Add to your init.lua or source this file: require('zeta-lsp')

-- 1. Filetype detection
vim.filetype.add({
  extension = {
    zl = 'zeta',
    zeta = 'zeta',
  },
})

-- 2. Treesitter parser (optional, install with :TSInstall zeta)
-- If no treesitter grammar exists, the LSP handles everything

-- 3. LSP Configuration (Neovim 0.11+ native)
vim.lsp.config('zeta', {
  cmd = { 'zeta-lsp' },
  filetypes = { 'zeta' },
  root_markers = { '.git' },
  capabilities = vim.lsp.protocol.make_client_capabilities(),
})

vim.lsp.enable('zeta')

-- 4. Keymaps (attach when LSP connects)
vim.api.nvim_create_autocmd('LspAttach', {
  callback = function(args)
    local bufnr = args.buf
    local opts = { buffer = bufnr, silent = true }

    -- Navigation
    vim.keymap.set('n', 'gd', vim.lsp.buf.definition, opts)
    vim.keymap.set('n', 'gD', vim.lsp.buf.declaration, opts)
    vim.keymap.set('n', 'gr', vim.lsp.buf.references, opts)
    vim.keymap.set('n', 'gi', vim.lsp.buf.implementation, opts)

    -- Info
    vim.keymap.set('n', 'K', vim.lsp.buf.hover, opts)
    vim.keymap.set('n', '<leader>rn', vim.lsp.buf.rename, opts)
    vim.keymap.set('n', '<leader>ca', vim.lsp.buf.code_action, opts)

    -- Diagnostics
    vim.keymap.set('n', '<leader>d', vim.diagnostic.open_float, opts)
    vim.keymap.set('n', '[d', vim.diagnostic.goto_prev, opts)
    vim.keymap.set('n', ']d', vim.diagnostic.goto_next, opts)
  end,
})

-- 5. Diagnostics styling
vim.diagnostic.config({
  virtual_text = { prefix = '●', spacing = 2 },
  signs = true,
  underline = true,
  update_in_insert = false,
  severity_sort = true,
  float = {
    border = 'rounded',
    source = 'always',
  },
})

-- 6. Completion (native, triggered by LSP)
vim.api.nvim_create_autocmd('LspAttach', {
  callback = function(args)
    vim.lsp.completion.enable(true, args.data.client_id, args.buf)
  end,
})

vim.o.completeopt = 'menu,menuone,noselect'

print('[zeta] Language support loaded')
