pipeline {
    agent any
    
    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }
        stage('Build') {
            steps {
                bat '''
                mkdir build 2>nul
                cd build
                cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_FLAGS="/W3"
                cmake --build . -- /p:WarningLevel=3
                '''
            }
        }
        stage('Git Info') {
            steps {
                script {
                    // 获取当前提交信息
                    def gitCommit = bat(script: 'git rev-parse HEAD', returnStdout: true).trim()
                    def gitAuthor = bat(script: 'git show -s --format=%%an', returnStdout: true).trim()
                    def gitEmail = bat(script: 'git show -s --format=%%ae', returnStdout: true).trim()
                    
                    echo "Commit: ${gitCommit}"
                    echo "Author: ${gitAuthor}"
                    echo "Email: ${gitEmail}"
                }
            }
        }
    }
    
    post {
        always {
            script {
                // 记录警告并获取结果
                def issues = recordIssues enabledForFailure: true,
                    tools: [msBuild()],
                    skipBlames: false
                
                // 获取新增警告
                def newIssues = issues.getNewIssues()
                
                if (newIssues.size() > 0) {
                    // 创建警告责任人映射
                    def authorWarnings = [:]
                    def authorEmails = [:]
                    
                    // 遍历新增警告
                    newIssues.each { issue ->
                        def author = issue.getAuthor() ?: "Unknown"
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
                        echo "Found author: ${author} with email: ${email} and ${warnings.size()} warnings"
                    }
                    
                    // 为每个责任人准备邮件内容，但只发送给测试邮箱
                    authorWarnings.each { author, warnings ->
                        def email = authorEmails[author]
                        def warningCount = warnings.size()
                        def subject = "[Jenkins] ${warningCount} new warning(s) in ${env.JOB_NAME} by ${author}"
                        
                        // 构建HTML邮件内容
                        def body = """
                        <html>
                        <head>
                            <style>
                                body { font-family: Arial, sans-serif; }
                                table { border-collapse: collapse; width: 100%; }
                                th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
                                th { background-color: #f2f2f2; }
                                tr:nth-child(even) { background-color: #f9f9f9; }
                                .warning { color: orange; }
                                .error { color: red; }
                            </style>
                        </head>
                        <body>
                            <h2>New Warnings Report</h2>
                            <p>Hello ${author},</p>
                            <p>You have introduced ${warningCount} new warning(s) in the project <b>${env.JOB_NAME}</b> (build #${env.BUILD_NUMBER}).</p>
                            <p>Please review and fix these warnings:</p>
                            <table>
                                <tr>
                                    <th>File</th>
                                    <th>Line</th>
                                    <th>Severity</th>
                                    <th>Message</th>
                                </tr>
                        """
                        
                        warnings.each { warning ->
                            def severityClass = warning.severity.toString().toLowerCase() == "error" ? "error" : "warning"
                            body += """
                                <tr>
                                    <td>${warning.file}</td>
                                    <td>${warning.line}</td>
                                    <td class="${severityClass}">${warning.severity}</td>
                                    <td>${warning.message}</td>
                                </tr>
                            """
                        }
                        
                        body += """
                            </table>
                            <p>View details in <a href="${env.BUILD_URL}warnings/">Jenkins Warnings Report</a>.</p>
                            <p>This is an automated message. Please do not reply.</p>
                        </body>
                        </html>
                        """
                        
                        // 只发送给测试邮箱，而不是实际责任人
                        // 注意：这里检查是否是您的邮箱，如果是则发送，否则只记录日志
                        if (email.toLowerCase() == "davexxx@163.com") {
                            emailext (
                                subject: subject,
                                body: body,
                                to: email,
                                mimeType: 'text/html',
                                attachLog: false
                            )
                            echo "Sent warning notification to ${author} <${email}> with ${warningCount} warnings"
                        } else {
                            // 其他责任人的邮件发送代码（已注释）
                            /* 
                            emailext (
                                subject: subject,
                                body: body,
                                to: email,
                                mimeType: 'text/html',
                                attachLog: false
                            )
                            */
                            echo "Would send warning notification to ${author} <${email}> with ${warningCount} warnings (SKIPPED - not test email)"
                        }
                    }
                } else {
                    echo "No new warnings found"
                }
            }
        }
    }
}
