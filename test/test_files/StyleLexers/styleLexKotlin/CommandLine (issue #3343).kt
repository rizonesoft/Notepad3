/*
 * Use of this source code is governed by the MIT license that can be
 * found in the LICENSE file.
 */

package org.rust.cargo.toolchain

import com.intellij.execution.ProgramRunnerUtil
import com.intellij.execution.RunManagerEx
import com.intellij.execution.RunnerAndConfigurationSettings
import com.intellij.execution.configuration.EnvironmentVariablesData
import com.intellij.execution.executors.DefaultRunExecutor
import org.rust.cargo.project.model.CargoProject
import org.rust.cargo.project.model.cargoProjects
import org.rust.cargo.project.workspace.CargoWorkspace
import org.rust.cargo.runconfig.command.workingDirectory
import org.rust.cargo.runconfig.createCargoCommandRunConfiguration
import org.rust.cargo.runconfig.wasmpack.WasmPackCommandConfiguration
import org.rust.cargo.runconfig.wasmpack.WasmPackCommandConfigurationType
import org.rust.stdext.buildList
import java.io.File
import java.nio.file.Path

abstract class RsCommandLineBase {
    abstract val command: String
    abstract val workingDirectory: Path
    abstract val redirectInputFrom: File?
    abstract val additionalArguments: List<String>

    protected abstract fun createRunConfiguration(runManager: RunManagerEx, name: String? = null): RunnerAndConfigurationSettings

    fun run(cargoProject: CargoProject, presentableName: String = command, saveConfiguration: Boolean = true) {
        val project = cargoProject.project
        val configurationName = when {
            project.cargoProjects.allProjects.size > 1 -> "$presentableName [${cargoProject.presentableName}]"
            else -> presentableName
        }
        val runManager = RunManagerEx.getInstanceEx(project)
        val configuration = createRunConfiguration(runManager, configurationName).apply {
            if (saveConfiguration) {
                runManager.setTemporaryConfiguration(this)
            }
        }
        val executor = DefaultRunExecutor.getRunExecutorInstance()
        ProgramRunnerUtil.executeConfiguration(configuration, executor)
    }
}

data class CargoCommandLine(
    override val command: String, // Can't be `enum` because of custom subcommands
    override val workingDirectory: Path, // Note that working directory selects Cargo project as well
    override val additionalArguments: List<String> = emptyList(),
    override val redirectInputFrom: File? = null,
    val backtraceMode: BacktraceMode = BacktraceMode.DEFAULT,
    val channel: RustChannel = RustChannel.DEFAULT,
    val environmentVariables: EnvironmentVariablesData = EnvironmentVariablesData.DEFAULT,
    val requiredFeatures: Boolean = true,
    val allFeatures: Boolean = false,
    val emulateTerminal: Boolean = false
) : RsCommandLineBase() {

    override fun createRunConfiguration(runManager: RunManagerEx, name: String?): RunnerAndConfigurationSettings =
        runManager.createCargoCommandRunConfiguration(this, name)

    /**
     * Adds [arg] to [additionalArguments] as an positional argument, in other words, inserts [arg] right after
     * `--` argument in [additionalArguments].
     * */
    fun withPositionalArgument(arg: String): CargoCommandLine {
        val (pre, post) = splitOnDoubleDash()
        if (arg in post) return this
        return copy(additionalArguments = pre + "--" + arg + post)
    }

    /**
     * Splits [additionalArguments] into parts before and after `--`.
     * For `cargo run --release -- foo bar`, returns (["--release"], ["foo", "bar"])
     */
    fun splitOnDoubleDash(): Pair<List<String>, List<String>> =
        org.rust.cargo.util.splitOnDoubleDash(additionalArguments)

    fun prependArgument(arg: String): CargoCommandLine =
        copy(additionalArguments = listOf(arg) + additionalArguments)

    companion object {
        fun forTargets(
            targets: List<CargoWorkspace.Target>,
            command: String,
            additionalArguments: List<String> = emptyList(),
            usePackageOption: Boolean = true
        ): CargoCommandLine {
            val pkgs = targets.map { it.pkg }
            // Make sure the selection does not span more than one package.
            assert(pkgs.map { it.rootDirectory }.distinct().size == 1)
            val pkg = pkgs.first()

            val targetArgs = targets.distinctBy { it.name }.flatMap { target ->
                when (target.kind) {
                    CargoWorkspace.TargetKind.Bin -> listOf("--bin", target.name)
                    CargoWorkspace.TargetKind.Test -> listOf("--test", target.name)
                    CargoWorkspace.TargetKind.ExampleBin, is CargoWorkspace.TargetKind.ExampleLib ->
                        listOf("--example", target.name)
                    CargoWorkspace.TargetKind.Bench -> listOf("--bench", target.name)
                    is CargoWorkspace.TargetKind.Lib -> listOf("--lib")
                    CargoWorkspace.TargetKind.CustomBuild,
                    CargoWorkspace.TargetKind.Unknown -> emptyList()
                }
            }

            val workingDirectory = if (usePackageOption) {
                pkg.workspace.contentRoot
            } else {
                pkg.rootDirectory
            }

            val commandLineArguments = buildList<String> {
                if (usePackageOption) {
                    add("--package")
                    add(pkg.name)
                }
                addAll(targetArgs)
                addAll(additionalArguments)
            }

            return CargoCommandLine(command, workingDirectory, commandLineArguments)
        }

        fun forTarget(
            target: CargoWorkspace.Target,
            command: String,
            additionalArguments: List<String> = emptyList(),
            usePackageOption: Boolean = true
        ): CargoCommandLine = forTargets(listOf(target), command, additionalArguments, usePackageOption)

        fun forProject(
            cargoProject: CargoProject,
            command: String,
            additionalArguments: List<String> = emptyList(),
            channel: RustChannel = RustChannel.DEFAULT
        ): CargoCommandLine = CargoCommandLine(
            command,
            workingDirectory = cargoProject.workingDirectory,
            additionalArguments = additionalArguments,
            channel = channel
        )

        fun forPackage(
            cargoPackage: CargoWorkspace.Package,
            command: String,
            additionalArguments: List<String> = emptyList()
        ): CargoCommandLine = CargoCommandLine(
            command,
            workingDirectory = cargoPackage.workspace.manifestPath.parent,
            additionalArguments = listOf("--package", cargoPackage.name) + additionalArguments
        )
    }
}

data class WasmPackCommandLine(
    override val command: String,
    override val workingDirectory: Path,
    override val additionalArguments: List<String> = emptyList()
) : RsCommandLineBase() {
    override val redirectInputFrom: File? = null

    override fun createRunConfiguration(runManager: RunManagerEx, name: String?): RunnerAndConfigurationSettings {
        val runnerAndConfigurationSettings = runManager.createConfiguration(
            name ?: command,
            WasmPackCommandConfigurationType.getInstance().factory
        )
        val configuration = runnerAndConfigurationSettings.configuration as WasmPackCommandConfiguration
        configuration.setFromCmd(this)
        return runnerAndConfigurationSettings
    }
}
