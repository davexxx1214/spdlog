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
    }
    
    post {
        always {
            recordIssues enabledForFailure: true,
                tools: [msBuild()],
                skipBlames: false,  // 替换 blameDisabled
                skipForensics: false  // 替换 forensicsDisabled
        }
    }
}
