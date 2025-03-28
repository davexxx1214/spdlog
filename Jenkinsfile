pipeline {
    agent any

    stages {
        stage('Checkout') {
            steps {
                echo "Workspace checked out by Jenkins SCM step."
            }
        }
        stage('Build') {
            steps {
                bat '''
                @echo off
                echo --- Creating build directory ---
                mkdir build 2>nul
                cd build
                if %errorlevel% neq 0 (
                    echo Failed to change directory to build.
                    exit /b 1
                )

                echo --- Running CMake configuration ---
                cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_FLAGS="/W3"
                if %errorlevel% neq 0 (
                    echo CMake configuration failed.
                    exit /b 1
                )

                echo --- Running CMake build ---
                cmake --build . -- /p:WarningLevel=3
                if %errorlevel% neq 0 (
                    echo CMake build failed.
                    exit /b 1
                )
                echo --- Build finished ---
                '''
            }
        }
        stage('Git Info') {
            steps {
                script {
                    try {
                        def gitCommit = bat(script: 'git rev-parse HEAD', returnStdout: true).trim()
                        def gitAuthor = bat(script: 'git show -s --format=%%an', returnStdout: true).trim()
                        def gitEmail = bat(script: 'git show -s --format=%%ae', returnStdout: true).trim()

                        echo "Commit: ${gitCommit}"
                        echo "Author: ${gitAuthor}"
                        echo "Email: ${gitEmail}"
                    } catch (Exception e) {
                        echo "Warning: Could not retrieve Git info. Error: ${e.getMessage()}"
                    }
                }
            }
        }
    }

    post {
        always {
            script {
                echo "--- Starting Post Build Actions ---"
                // 1. 执行 recordIssues，记录 MSBuild 警告，并指定源文件编码
                recordIssues enabledForFailure: true,
                             tools: [msBuild()],
                             skipBlames: false, // 尝试获取作者信息
                             sourceCodeEncoding: 'UTF-8' // 修正编码问题

                echo "Searching for Warnings NG ResultAction..."
                // 2. 使用 getRawBuild() 获取底层构建对象再访问 actions (需要管理员批准 getRawBuild)
                def rawBuild = currentBuild.getRawBuild()
                if (rawBuild != null) { // 添加检查确保 rawBuild 不是 null
                    // 使用 rawBuild.actions 查找 ResultAction
                    def action = rawBuild.actions.find { it instanceof io.jenkins.plugins.analysis.core.model.ResultAction }

                    if (action) {
                        echo "Warnings NG ResultAction found. Processing issues..."

                        // 3. 尝试从 Action 对象直接获取新问题的数量 (适用于旧版 Warnings NG)
                        // 因为 getNewIssues() 方法在旧版本中可能不存在
                        def newSize = 0
                        boolean newSizeObtained = false
                        try {
                            // 优先尝试 getNewSize() 方法 (需要 Script Security 批准)
                            newSize = action.getNewSize()
                            newSizeObtained = true
                            echo "Found ${newSize} new issues (via getNewSize() method)."
                        } catch (MissingMethodException mme) {
                            echo "WARN: getNewSize() method not found. Trying direct 'newSize' property access..."
                            try {
                                // 如果 getNewSize() 不行，尝试直接访问 newSize 属性 (需要 Script Security 批准)
                                newSize = action.newSize
                                newSizeObtained = true
                                echo "Found ${newSize} new issues (via newSize property)."
                            } catch (MissingPropertyException mpe) {
                                echo "ERROR: Could not get new issue count. Neither getNewSize() method nor newSize property is available: ${mpe.getMessage()}"
                            }
                        } catch (Exception e) {
                             echo "ERROR: An unexpected error occurred while trying to get new issue count: ${e.getMessage()}"
                        }

                        // 只有成功获取到新问题数量才继续
                        if (newSizeObtained) {
                            if (newSize > 0) {
                                echo "Processing ${newSize} new issues..."
                                // !!! 重要限制 !!!
                                // 这个方法只能获取新问题的 *数量*，无法获取详细列表 (作者, 文件, 行号等)。
                                // 因此，邮件通知被简化，不再按作者发送详细信息。

                                // 简化版通知：只通知有新警告，但不列出详情
                                def subject = "[Jenkins Warning] ${newSize} new warning(s) in ${env.JOB_NAME} build #${env.BUILD_NUMBER}"
                                def body = """
                                <html>
                                <head>
                                    <style>body { font-family: Arial, sans-serif; }</style>
                                </head>
                                <body>
                                    <h2>New Warnings Report</h2>
                                    <p>Hello,</p>
                                    <p><b>${newSize}</b> new warning(s) were detected in the project <b>${env.JOB_NAME}</b> (build <a href="${env.BUILD_URL}">${env.BUILD_NUMBER}</a>).</p>
                                    <p>Please check the build results for details:</p>
                                    <p><a href="${env.BUILD_URL}warnings/">${env.BUILD_URL}warnings/</a></p>
                                    <p><i>This is an automated message.</i></p>
                                </body>
                                </html>
                                """

                                // --- 邮件发送逻辑 (发送给固定地址) ---
                                // !!! 修改这里的收件人地址 !!!
                                def recipient = "davexxx@163.com"
                                try {
                                    echo "Sending simplified warning notification to ${recipient}..."
                                    emailext (
                                        subject: subject,
                                        body: body,
                                        to: recipient,
                                        mimeType: 'text/html',
                                        attachLog: false
                                    )
                                    echo "Successfully sent simplified notification."
                                } catch (e) {
                                    echo "ERROR: Failed to send simplified email to ${recipient}: ${e.getMessage()}"
                                    // 考虑标记构建不稳定
                                    // currentBuild.result = 'UNSTABLE'
                                }
                                // --- 邮件发送逻辑结束 ---

                            } else {
                                echo "No new warnings found. No notifications needed."
                            }
                        } else {
                             // 如果连 newSize 都获取不到，标记构建不稳定
                             echo "Could not determine the number of new issues. Marking build as UNSTABLE."
                             currentBuild.result = 'UNSTABLE'
                        }

                    } else {
                        echo "WARNING: Warnings NG result action not found. Cannot process issues or send notifications."
                        // 可能需要检查 Warnings NG 插件是否正确安装和配置
                        // currentBuild.result = 'UNSTABLE'
                    }
                } else {
                    echo "ERROR: Could not get raw build object from currentBuild. Cannot access actions."
                    // 如果 getRawBuild() 返回 null，则无法继续，标记构建不稳定
                    currentBuild.result = 'UNSTABLE'
                }
                echo "--- Finished Post Build Actions ---"
            }
        }
    }
}
