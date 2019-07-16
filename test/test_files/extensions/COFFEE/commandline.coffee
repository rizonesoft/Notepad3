###
CoffeeLint

Copyright (c) 2011 Matthew Perpick.
CoffeeLint is freely distributable under the MIT license.
###


resolve = require('resolve').sync
path = require('path')
fs   = require('fs')
os   = require('os')
glob = require('glob')
optimist = require('optimist')
ignore = require('ignore')
stripComments = require('strip-json-comments')
thisdir = path.dirname(fs.realpathSync(__filename))
coffeelint = require(path.join(thisdir, 'coffeelint'))
configfinder = require(path.join(thisdir, 'configfinder'))
ruleLoader = require(path.join(thisdir, 'ruleLoader'))
Cache = require(path.join(thisdir, 'cache'))
CoffeeScript = require 'coffee-script'
CoffeeScript.register()

log = ->
    # coffeelint: disable=no_debugger
    console.log(arguments...)
    # coffeelint: enable=no_debugger

jsonIndentation = 2
logConfig = (config) ->
    filter = (k, v) -> v unless k in ['message', 'description', 'name']
    log(JSON.stringify(config, filter, jsonIndentation))

# Return the contents of the given file synchronously.
read = (path) ->
    realPath = fs.realpathSync(path)
    return fs.readFileSync(realPath).toString()

# build all extentions to search
getAllExtention = (extension) ->
    if extension?
        extension = ['coffee'].concat(extension?.split(','))
        "@(#{extension.join('|')})"
    else
        'coffee'

# Return a list of CoffeeScript's in the given paths.
findCoffeeScripts = (paths, extension) ->
    files = []
    allExtention = getAllExtention(extension)
    for p in paths
        if fs.statSync(p).isDirectory()
            # The glob library only uses forward slashes.
            files = files.concat(glob.sync("#{p}/**/*.#{allExtention}"))
        else
            files.push(p)
    return files

# Return an error report from linting the given paths.
lintFiles = (files, config) ->


    errorReport = new coffeelint.getErrorReport()
    for file in files
        source = read(file)
        literate = CoffeeScript.helpers.isLiterate file

        fileConfig = if config then config else getFallbackConfig(file)

        errorReport.lint(file, source, fileConfig, literate)
    return errorReport

# Return an error report from linting the given coffeescript source.
lintSource = (source, config, literate = false) ->
    errorReport = new coffeelint.getErrorReport()
    config or= getFallbackConfig()

    errorReport.lint('stdin', source, config, literate)
    return errorReport

# Load a config file given a path/filename
readConfigFile = (path) ->
    text = read(path)
    try
        jsonIndentation = text.split('\n')[1].match(/^\s+/)[0].length
    JSON.parse(stripComments(text))

loadConfig = (options) ->
    config = null
    unless options.argv.noconfig
        if options.argv.f
            config = readConfigFile(options.argv.f)

            # If -f was specifying a package.json, extract the config
            if config.coffeelintConfig
                config = config.coffeelintConfig
    config

# Get fallback configuration. With the -F flag found configs in standard places
# will be used for each file being linted. Standard places are package.json or
# coffeelint.json in a project's root folder or the user's home folder.
getFallbackConfig = (filename = null) ->
    unless options.argv.noconfig
        configfinder.getConfig(filename)

# These reporters are usually parsed by other software, so I can't just echo a
# warning.  Creating a fake file is my best attempt.
deprecatedReporter = (errorReport, reporter) ->
    errorReport.paths['coffeelint_fake_file.coffee'] ?= []
    errorReport.paths['coffeelint_fake_file.coffee'].push {
        'level': 'warn'
        'rule': 'commandline'
        'message': "parameter --#{reporter} is deprecated.
            Use --reporter #{reporter} instead"
        'lineNumber': 0
    }
    return reporter

coreReporters =
    default: require(path.join(thisdir, 'reporters', 'default'))
    csv: require(path.join(thisdir, 'reporters', 'csv'))
    jslint: require(path.join(thisdir, 'reporters', 'jslint'))
    checkstyle: require(path.join(thisdir, 'reporters', 'checkstyle'))
    raw: require(path.join(thisdir, 'reporters', 'raw'))

# Publish the error report and exit with the appropriate status.
reportAndExit = (errorReport, options) ->
    strReporter = if options.argv.jslint
        deprecatedReporter(errorReport, 'jslint')
    else if options.argv.csv
        deprecatedReporter(errorReport, 'csv')
    else if options.argv.checkstyle
        deprecatedReporter(errorReport, 'checkstyle')
    else
        options.argv.reporter

    strReporter ?= 'default'
    SelectedReporter = coreReporters[strReporter] ? do ->
        try
            reporterPath = resolve strReporter, {
                basedir: process.cwd()
            }
        catch
            reporterPath = strReporter
        require reporterPath

    options.argv.color ?= if options.argv.nocolor then 'never' else 'auto'

    colorize = switch options.argv.color
        when 'always' then true
        when 'never' then false
        else process.stdout.isTTY

    reporter = new SelectedReporter errorReport, {
        colorize: colorize
        quiet: options.argv.q
    }
    reporter.publish()

    process.on 'exit', () ->
        process.exit errorReport.getExitCode()

# Declare command line options.
options = optimist
            .usage('Usage: coffeelint [options] source [...]')
            .alias('f', 'file')
            .alias('h', 'help')
            .alias('v', 'version')
            .alias('s', 'stdin')
            .alias('q', 'quiet')
            .alias('c', 'cache')
            .describe('f', 'Specify a custom configuration file.')
            .describe('rules', 'Specify a custom rule or directory of rules.')
            .describe('makeconfig', 'Prints a default config file')
            .describe('trimconfig', 'Compares your config with the default and
                prints a minimal configuration')
            .describe('noconfig', 'Ignores any config file.')
            .describe('h', 'Print help information.')
            .describe('v', 'Print current version number.')
            .describe('r', '(not used, but left for backward compatibility)')
            .describe('reporter', 'built in reporter (default, csv, jslint,
                checkstyle, raw), or module, or path to reporter file.')
            .describe('csv', '[deprecated] use --reporter csv')
            .describe('jslint', '[deprecated] use --reporter jslint')
            .describe('nocolor', '[deprecated] use --color=never')
            .describe('checkstyle', '[deprecated] use --reporter checkstyle')
            .describe('color=<when>',
              'When to colorize the output. <when> can be one of always, never \
              , or auto.')
            .describe('s', 'Lint the source from stdin')
            .describe('q', 'Only print errors.')
            .describe('literate',
                'Used with --stdin to process as Literate CoffeeScript')
            .describe('c', 'Cache linting results')
            .describe('ext',
                'Specify an additional file extension, separated by comma.')
            .boolean('csv')
            .boolean('jslint')
            .boolean('checkstyle')
            .boolean('nocolor')
            .boolean('noconfig')
            .boolean('makeconfig')
            .boolean('trimconfig')
            .boolean('literate')
            .boolean('r')
            .boolean('s')
            .boolean('q', 'Print errors only.')
            .boolean('c')

if options.argv.v
    log coffeelint.VERSION
    process.exit(0)
else if options.argv.h
    options.showHelp()
    process.exit(0)
else if options.argv.trimconfig
    userConfig = loadConfig(options) ? getFallbackConfig()
    logConfig(coffeelint.trimConfig(userConfig))

else if options.argv.makeconfig
    logConfig(coffeelint.getRules())
else if options.argv._.length < 1 and not options.argv.s
    options.showHelp()
    process.exit(1)

else
    # Initialize cache, if enabled
    if options.argv.cache
        coffeelint.setCache new Cache(path.join(os.tmpdir(), 'coffeelint'))

    # Load configuration.
    config = loadConfig(options)

    ruleLoader.loadRule(coffeelint, options.argv.rules) if options.argv.rules

    if options.argv.s
        # Lint from stdin
        data = ''
        stdin = process.openStdin()
        stdin.on 'data', (buffer) ->
            data += buffer.toString() if buffer
        stdin.on 'end', ->
            errorReport = lintSource(data, config, options.argv.literate)
            reportAndExit errorReport, options
    else
        # Find scripts to lint.
        paths = options.argv._
        scripts = findCoffeeScripts(paths, options.argv.ext)
        scripts = ignore().addIgnoreFile('.coffeelintignore').filter(scripts)

        # Lint the code.
        errorReport = lintFiles(scripts, config, options.argv.literate)
        reportAndExit errorReport, options
