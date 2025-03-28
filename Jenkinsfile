pipeline {
    agent any

    stages {
        stage('Checkout') {
            steps {
                // Jenkins 会自动处理初始 checkout，但如果需要特定分支或行为，可以在这里明确 checkout
                // checkout scm
                // 如果你的 pipeline 是多分支类型，通常第一步的 checkout 是隐式的
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
                        // 获取当前提交信息
                        def gitCommit = bat(script: 'git rev-parse HEAD', returnStdout: true).trim()
                        def gitAuthor = bat(script: 'git show -s --format=%%an', returnStdout: true).trim()
                        def gitEmail = bat(script: 'git show -s --format=%%ae', returnStdout: true).trim()

                        echo "Commit: ${gitCommit}"
                        echo "Author: ${gitAuthor}"
                        echo "Email: ${gitEmail}"
                    } catch (Exception e) {
                        echo "Warning: Could not retrieve Git info. Error: ${e.getMessage()}"
                        // 可以选择让构建失败或继续
                        // currentBuild.result = 'UNSTABLE'
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

                // 2. 在 recordIssues 执行后，查找 Warnings NG 插件添加的 ResultAction
                // 需要确保 Warnings Next Generation 插件已安装
                echo "Searching for Warnings NG ResultAction..."
                def action = currentBuild.actions.find { it instanceof io.jenkins.plugins.analysis.core.model.ResultAction }

                if (action) {
                    echo "Warnings NG ResultAction found. Processing issues..."
                    // 3. 从 Action 对象获取新问题
                    def newIssues = action.getNewIssues()
                    echo "Found ${newIssues.size()} new issues."

                    if (newIssues.size() > 0) {
                        // 创建警告责任人映射
                        def authorWarnings = [:]
                        def authorEmails = [:]

                        // 遍历新增警告
                        newIssues.each { issue ->
                            // 尝试获取作者信息，如果 blame 失败可能为空
                            def author = issue.getAuthorName() ?: "Unknown"
                            def email = issue.getAuthorEmail() ?: "unknown@example.com"

                            if (!authorWarnings.containsKey(author)) {
                                authorWarnings[author] = []
                                authorEmails[author] = email
                            }

                            authorWarnings[author] << [
                                file: issue.getFileName(),
                                line: issue.getLineStart(),
                                message: issue.getMessage(),
                                severity: issue.getSeverity()
                            ]
                        }

                        // 输出找到的责任人信息（用于调试）
                        authorWarnings.each { author, warnings ->
                            def email = authorEmails[author]
                            echo "Debugging - Author: ${author}, Email: ${email}, Warnings Count: ${warnings.size()}"
                        }

                        // 为每个责任人准备邮件内容
                        authorWarnings.each { author, warnings ->
                            def email = authorEmails[author]
                            // 如果 email 是 "unknown@example.com"，可能意味着 git blame 失败或未配置
                            if (email == "unknown@example.com" || email.trim().isEmpty()) {
                                echo "Skipping email for author '${author}' due to unknown or empty email address."
                                return // continue to next author in each loop
                            }

                            def warningCount = warnings.size()
                            def subject = "[Jenkins Warning] ${warningCount} new warning(s) in ${env.JOB_NAME} build #${env.BUILD_NUMBER} by ${author}"

                            // 构建HTML邮件内容
                            def body = """
                            <html>
                            <head>
                                <style>
                                    body { font-family: Arial, sans-serif; font-size: 14px; }
                                    table { border-collapse: collapse; width: 100%; margin-top: 15px; }
                                    th, td { border: 1px solid #ddd; padding: 8px; text-align: left; word-wrap: break-word; }
                                    th { background-color: #f2f2f2; }
                                    tr:nth-child(even) { background-color: #f9f9f9; }
                                    .severity-high, .severity-error { color: red; font-weight: bold; }
                                    .severity-normal, .severity-warning { color: orange; }
                                    .severity-low { color: gray; }
                                    h2 { color: #333; }
                                    p { line-height: 1.6; }
                                    a { color: #007bff; text-decoration: none; }
                                    a:hover { text-decoration: underline; }
                                </style>
                            </head>
                            <body>
                                <h2>New Warnings Report</h2>
                                <p>Hello ${author},</p>
                                <p>You have introduced <b>${warningCount}</b> new warning(s) in the project <b>${env.JOB_NAME}</b> (build <a href="${env.BUILD_URL}">${env.BUILD_NUMBER}</a>).</p>
                                <p>Please review and address these issues:</p>
                                <table>
                                    <tr>
                                        <th>File</th>
                                        <th>Line</th>
                                        <th>Severity</th>
                                        <th>Message</th>
                                    </tr>
                            """

                            warnings.each { warning ->
                                // 尝试将 Severity 转换为小写以匹配 CSS 类
                                def severityText = warning.severity.toString()
                                def severityClass = "severity-${severityText.toLowerCase()}"
                                body += """
                                    <tr>
                                        <td>${warning.file}</td>
                                        <td>${warning.line}</td>
                                        <td class="${severityClass}">${severityText}</td>
                                        <td>${warning.message}</td>
                                    </tr>
                                """
                            }

                            body += """
                                </table>
                                <p>View the full report in Jenkins: <a href="${env.BUILD_URL}warnings/">${env.BUILD_URL}warnings/</a></p>
                                <p><i>This is an automated message.</i></p>
                            </body>
                            </html>
                            """

                            // --- 邮件发送逻辑 ---
                            // 修改这里的条件以控制邮件发送
                            def sendToTestEmailOnly = true // 设置为 false 以发送给实际责任人
                            def testEmail = "davexxx@163.com" // 你的测试邮箱

                            if (sendToTestEmailOnly) {
                                if (email.toLowerCase() == testEmail.toLowerCase()) {
                                    try {
                                        echo "Sending warning notification to TEST EMAIL for author ${author} <${email}>..."
                                        emailext (
                                            subject: subject,
                                            body: body,
                                            to: testEmail, // 始终发送到测试邮箱
                                            mimeType: 'text/html',
                                            attachLog: false
                                        )
                                        echo "Successfully sent test email for ${author}."
                                    } catch (e) {
                                        echo "ERROR: Failed to send test email for ${author} to ${testEmail}: ${e.getMessage()}"
                                        // 可以考虑标记构建为 UNSTABLE
                                        // currentBuild.result = 'UNSTABLE'
                                    }
                                } else {
                                    echo "SKIPPED sending email for author ${author} <${email}> (Test mode: Only sending to ${testEmail})"
                                }
                            } else {
                                // 发送给实际责任人
                                try {
                                    echo "Sending warning notification to actual author ${author} <${email}>..."
                                    emailext (
                                        subject: subject,
                                        body: body,
                                        to: email, // 发送给实际检测到的邮箱
                                        mimeType: 'text/html',
                                        attachLog: false
                                    )
                                    echo "Successfully sent email to ${author} <${email}>."
                                } catch (e) {
                                    echo "ERROR: Failed to send email to ${author} <${email}>: ${e.getMessage()}"
                                    // currentBuild.result = 'UNSTABLE'
                                }
                            }
                            // --- 邮件发送逻辑结束 ---
                        }
                    } else {
                        echo "No new warnings found. No notifications needed."
                    }
                } else {
                    echo "WARNING: Warnings NG result action not found. Cannot process issues or send notifications."
                    // 可能需要检查 Warnings NG 插件是否正确安装和配置
                    // currentBuild.result = 'UNSTABLE'
                }
                echo "--- Finished Post Build Actions ---"
            }
        }
    }
}
