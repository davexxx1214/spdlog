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
                cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_FLAGS="/W4 /WX-"
                cmake --build . -- /p:WarningLevel=4 "/p:TreatWarningAsError=false"
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
            recordIssues enabledForFailure: true,
                tools: [msBuild()],
                skipBlames: false
        }
    }
}
