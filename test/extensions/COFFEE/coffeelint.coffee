###
CoffeeLint

Copyright (c) 2011 Matthew Perpick.
CoffeeLint is freely distributable under the MIT license.
###

# Coffeelint's namespace.
# Browserify wrapps this file in a UMD that will set window.coffeelint to
# exports
coffeelint = exports

# Hide from browserify
nodeRequire = require

if window?
    # If we're in the browser assume CoffeeScript is already loaded.
    CoffeeScript = window.CoffeeScript
# By using nodeRequire it prevents browserify from finding this dependency.
# If it isn't hidden there is an error attempting to inline CoffeeScript.
# if browserify uses `-i` to ignore the dependency it creates an empty shim
# which breaks NodeJS
# https://github.com/substack/node-browserify/issues/471
#
# Atom has a `window`, but not a `window.CoffeeScript`. Calling `nodeRequire`
# here should fix Atom without breaking anything else.
CoffeeScript ?= nodeRequire 'coffee-script'

unless CoffeeScript?
    throw new Error('Unable to find CoffeeScript')

# Browserify will inline the file at compile time.
packageJSON = require('./../package.json')

# The current version of Coffeelint.
coffeelint.VERSION = packageJSON.version

# CoffeeLint error levels.
ERROR   = 'error'
WARN    = 'warn'
IGNORE  = 'ignore'

coffeelint.RULES = RULES = require './rules.coffee'

# Patch the source properties onto the destination.
extend = (destination, sources...) ->
    for source in sources
        (destination[k] = v for k, v of source)
    return destination

# Patch any missing attributes from defaults to source.
defaults = (source, defaults) ->
    extend({}, defaults, source)

# Helper to remove rules from disabled list
difference = (a, b) ->
    j = 0
    while j < a.length
        if a[j] in b
            a.splice(j, 1)
        else
            j++

LineLinter = require './line_linter.coffee'
LexicalLinter = require './lexical_linter.coffee'
ASTLinter = require './ast_linter.coffee'

# Cache instance, disabled by default
cache = null

# Merge default and user configuration.
mergeDefaultConfig = (userConfig) ->
    # When run from the browser it may not be able to find the ruleLoader.
    try
        ruleLoader = nodeRequire './ruleLoader'
        ruleLoader.loadFromConfig coffeelint, userConfig

    config = {}
    if userConfig.coffeelint
        config.coffeelint = userConfig.coffeelint
    for rule, ruleConfig of RULES
        config[rule] = defaults(userConfig[rule], ruleConfig)
    return config

sameJSON = (a, b) ->
    JSON.stringify(a) is JSON.stringify(b)

coffeelint.trimConfig = (userConfig) ->
    newConfig = {}
    userConfig = mergeDefaultConfig(userConfig)
    for rule, config of userConfig
        dConfig = RULES[rule]

        if rule is 'coffeelint'
            config.transforms = config._transforms
            delete config._transforms

            config.coffeescript = config._coffeescript
            delete config._coffeescript

            newConfig[rule] = config
        else if config.level is dConfig.level is 'ignore'
            # If the rule is going to be ignored and would be by default it
            # doesn't matter what you may have configured
            undefined
        else if config.level is 'ignore'
            # If the rule is being ignored you don't need the rest of the
            # config.
            newConfig[rule] = { level: 'ignore' }
        else
            config.module = config._module
            delete config._module
            for key, value of config
                continue if key in ['message', 'description', 'name']

                dValue = dConfig[key]
                if value isnt dValue and not sameJSON(value, dValue)
                    newConfig[rule] ?= {}
                    newConfig[rule][key] = value
    return newConfig

coffeelint.invertLiterate = (source) ->
    source = CoffeeScript.helpers.invertLiterate source
    # Strip the first 4 spaces from every line. After this the markdown is
    # commented and all of the other code should be at their natural location.
    newSource = ''
    for line in source.split '\n'
        if line.match(/^#/)
            # strip trailing space
            line = line.replace /\s*$/, ''
        # Strip the first 4 spaces of every line. This is how Markdown
        # indicates code, so in the end this pulls everything back to where it
        # would be indented if it hadn't been written in literate style.
        line = line.replace /^\s{4}/g, ''
        newSource += "#{line}\n"

    newSource

_rules = {}
coffeelint.registerRule = (RuleConstructor, ruleName = undefined) ->
    p = new RuleConstructor

    name = p?.rule?.name or '(unknown)'
    e = (msg) -> throw new Error "Invalid rule: #{name} #{msg}"
    unless p.rule?
        e 'Rules must provide rule attribute with a default configuration.'

    e 'Rule defaults require a name' unless p.rule.name?

    if ruleName? and ruleName isnt p.rule.name
        e "Mismatched rule name: #{ruleName}"

    e 'Rule defaults require a message' unless p.rule.message?
    e 'Rule defaults require a description' unless p.rule.description?
    unless p.rule.level in ['ignore', 'warn', 'error']
        e "Default level must be 'ignore', 'warn', or 'error'"

    if typeof p.lintToken is 'function'
        e "'tokens' is required for 'lintToken'" unless p.tokens
    else if typeof p.lintLine isnt 'function' and
            typeof p.lintAST isnt 'function'
        e 'Rules must implement lintToken, lintLine, or lintAST'

    # Capture the default options for the new rule.
    RULES[p.rule.name] = p.rule
    _rules[p.rule.name] = RuleConstructor

coffeelint.getRules = ->
    output = {}
    for key in Object.keys(RULES).sort()
        output[key] = RULES[key]
    output

# These all need to be explicitly listed so they get picked up by browserify.
coffeelint.registerRule require './rules/arrow_spacing.coffee'
coffeelint.registerRule require './rules/braces_spacing.coffee'
coffeelint.registerRule require './rules/no_tabs.coffee'
coffeelint.registerRule require './rules/no_trailing_whitespace.coffee'
coffeelint.registerRule require './rules/max_line_length.coffee'
coffeelint.registerRule require './rules/line_endings.coffee'
coffeelint.registerRule require './rules/no_trailing_semicolons.coffee'
coffeelint.registerRule require './rules/indentation.coffee'
coffeelint.registerRule require './rules/camel_case_classes.coffee'
coffeelint.registerRule require './rules/colon_assignment_spacing.coffee'
coffeelint.registerRule require './rules/no_implicit_braces.coffee'
coffeelint.registerRule require './rules/no_nested_string_interpolation.coffee'
coffeelint.registerRule require './rules/no_plusplus.coffee'
coffeelint.registerRule require './rules/no_throwing_strings.coffee'
coffeelint.registerRule require './rules/no_backticks.coffee'
coffeelint.registerRule require './rules/no_implicit_parens.coffee'
coffeelint.registerRule require './rules/no_empty_param_list.coffee'
coffeelint.registerRule require './rules/no_stand_alone_at.coffee'
coffeelint.registerRule require './rules/space_operators.coffee'
coffeelint.registerRule require './rules/duplicate_key.coffee'
coffeelint.registerRule require './rules/empty_constructor_needs_parens.coffee'
coffeelint.registerRule require './rules/cyclomatic_complexity.coffee'
coffeelint.registerRule require './rules/newlines_after_classes.coffee'
coffeelint.registerRule require './rules/no_unnecessary_fat_arrows.coffee'
coffeelint.registerRule require './rules/missing_fat_arrows.coffee'
coffeelint.registerRule(
    require './rules/non_empty_constructor_needs_parens.coffee'
)
coffeelint.registerRule require './rules/no_unnecessary_double_quotes.coffee'
coffeelint.registerRule require './rules/no_debugger.coffee'
coffeelint.registerRule(
    require './rules/no_interpolation_in_single_quotes.coffee'
)
coffeelint.registerRule require './rules/no_empty_functions.coffee'
coffeelint.registerRule require './rules/prefer_english_operator.coffee'
coffeelint.registerRule require './rules/spacing_after_comma.coffee'
coffeelint.registerRule(
    require './rules/transform_messes_up_line_numbers.coffee'
)
coffeelint.registerRule require './rules/ensure_comprehensions.coffee'
coffeelint.registerRule require './rules/no_this.coffee'
coffeelint.registerRule require './rules/eol_last.coffee'
coffeelint.registerRule require './rules/no_private_function_fat_arrows.coffee'

hasSyntaxError = (source) ->
    try
        # If there are syntax errors this will abort the lexical and line
        # linters.
        CoffeeScript.tokens(source)
        return false
    return true

ErrorReport = require('./error_report.coffee')
coffeelint.getErrorReport = ->
    new ErrorReport coffeelint

# Check the source against the given configuration and return an array
# of any errors found. An error is an object with the following
# properties:
#
#   {
#       rule :      'Name of the violated rule',
#       lineNumber: 'Number of the line that caused the violation',
#       level:      'The error level of the violated rule',
#       message:    'Information about the violated rule',
#       context:    'Optional details about why the rule was violated'
#   }
#
coffeelint.lint = (source, userConfig = {}, literate = false) ->
    errors = []

    cache?.setConfig userConfig
    if cache?.has source then return cache?.get source
    config = mergeDefaultConfig(userConfig)

    source = @invertLiterate source if literate
    if userConfig?.coffeelint?.transforms?
        sourceLength = source.split('\n').length
        for m in userConfig?.coffeelint?.transforms
            try
                ruleLoader = nodeRequire './ruleLoader'
                transform = ruleLoader.require(m)
                source = transform source

        # NOTE: This can have false negatives. For example if your transformer
        # changes one line into two early in the file and later condenses two
        # into one you'll end up with the same length and not get the warning
        # even though everything in between will be off by one.
        if sourceLength isnt source.split('\n').length and
                config.transform_messes_up_line_numbers.level isnt 'ignore'

            errors.push(extend(
                {
                    lineNumber: 1
                    context: "File was transformed from
                        #{sourceLength} lines to
                        #{source.split("\n").length} lines"
                },
                config.transform_messes_up_line_numbers
            ))

    if userConfig?.coffeelint?.coffeescript?
        CoffeeScript = ruleLoader.require userConfig.coffeelint.coffeescript

    # coffeescript_error is unique because it's embedded in the ASTLinter. It
    # indicates a syntax error and would not work well as a stand alone rule.
    #
    # Why can't JSON just support comments?
    for name of userConfig when name not in ['coffeescript_error', '_comment']
        unless _rules[name]?
            # TODO: Figure out a good way to notify the user that they have
            # configured a rule that doesn't exist. throwing an Error was
            # definitely a mistake. I probably need a good way to generate lint
            # warnings for configuration.
            undefined

    # disabledInitially is to prevent the rule from becoming active before
    # the actual inlined comment appears
    disabledInitially = []
    # Check ahead for inline enabled rules
    for l in source.split('\n')
        [ regex, set, rule ] = LineLinter.configStatement.exec(l) or []
        if set is 'enable' and config[rule]?.level is 'ignore'
            disabledInitially.push rule
            config[rule].level = 'error'

    # Do AST linting first so all compile errors are caught.
    astErrors = new ASTLinter(source, config, _rules, CoffeeScript).lint()
    errors = errors.concat(astErrors)

    # only do further checks if the syntax is okay, otherwise they just fail
    # with syntax error exceptions
    unless hasSyntaxError(source)
        # Do lexical linting.
        lexicalLinter = new LexicalLinter(source, config, _rules, CoffeeScript)
        lexErrors = lexicalLinter.lint()
        errors = errors.concat(lexErrors)

        # Do line linting.
        tokensByLine = lexicalLinter.tokensByLine
        lineLinter = new LineLinter(source, config, _rules, tokensByLine,
            literate)
        lineErrors = lineLinter.lint()
        errors = errors.concat(lineErrors)
        inlineConfig = lineLinter.inlineConfig
    else
        # default this so it knows what to do
        inlineConfig =
            enable: {}
            disable: {}

    # Sort by line number and return.
    errors.sort((a, b) -> a.lineNumber - b.lineNumber)

    # Disable/enable rules for inline blocks
    allErrors = errors
    errors = []
    disabled = disabledInitially
    nextLine = 0
    for i in [0...source.split('\n').length]
        for cmd of inlineConfig
            rules = inlineConfig[cmd][i]
            {
                'disable': ->
                    disabled = disabled.concat(rules)
                'enable': ->
                    difference(disabled, rules)
                    disabled = disabledInitially if rules.length is 0
            }[cmd]() if rules?
        # advance line and append relevant messages
        while nextLine is i and allErrors.length > 0
            nextLine = allErrors[0].lineNumber - 1
            e = allErrors[0]
            if e.lineNumber is i + 1 or not e.lineNumber?
                e = allErrors.shift()
                errors.push e unless e.rule in disabled

    cache?.set source, errors

    errors

coffeelint.setCache = (obj) -> cache = obj
