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
                // 1. 执行 recordIssues 以生成报告 (保留)
                //    即使我们无法在脚本中获取结果，这一步仍然会在 UI 中生成报告。
                try {
                    recordIssues enabledForFailure: true,
                                 tools: [msBuild()],
                                 skipBlames: false, // 尝试获取作者信息（报告中会显示）
                                 sourceCodeEncoding: 'UTF-8'
                    echo "recordIssues step completed."
                } catch (Exception e) {
                    echo "ERROR during recordIssues step: ${e.getMessage()}"
                    // 即使 recordIssues 失败，也可能希望继续发送邮件
                }


                // 2. 发送简单的构建完成通知邮件
                //    不再尝试获取新警告数量，只报告构建状态。
                echo "Preparing build status notification..."
                // 使用 currentBuild.currentResult 获取最终状态 (SUCCESS, UNSTABLE, FAILURE etc.)
                def finalStatus = currentBuild.currentResult ?: 'UNKNOWN'
                echo "Build finished with status: ${finalStatus}"

                def subject = "[Jenkins Build] ${env.JOB_NAME} #${env.BUILD_NUMBER} - ${finalStatus}"
                def body = """
                <html>
                <head>
                    <style>body { font-family: Arial, sans-serif; }</style>
                </head>
                <body>
                    <h2>Build Report</h2>
                    <p>Project: <b>${env.JOB_NAME}</b></p>
                    <p>Build Number: <a href="${env.BUILD_URL}">${env.BUILD_NUMBER}</a></p>
                    <p>Status: <b>${finalStatus}</b></p>
                    <p>Check the build console output and warnings report (if generated):</p>
                    <ul>
                        <li><a href="${env.BUILD_URL}console">Console Output</a></li>
                        <li><a href="${env.BUILD_URL}warnings/">Warnings Report</a> (Link active if warnings were recorded)</li>
                    </ul>
                    <p><i>This is an automated message.</i></p>
                </body>
                </html>
                """

                // --- 邮件发送逻辑 (发送给固定地址) ---
                // !!! 修改这里的收件人地址 !!!
                def recipient = "davexxx@163.com"
                try {
                    echo "Sending build status notification to ${recipient}..."
                    emailext (
                        subject: subject,
                        body: body,
                        to: recipient,
                        mimeType: 'text/html',
                        attachLog: false
                    )
                    echo "Successfully sent build status notification."
                } catch (e) {
                    echo "ERROR: Failed to send build status email to ${recipient}: ${e.getMessage()}"
                    // 邮件发送失败不应影响构建最终状态，只记录错误
                }
                // --- 邮件发送逻辑结束 ---

                echo "--- Finished Post Build Actions ---"
            }
        }
    }
}
