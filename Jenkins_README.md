# Tracking Code Warning Authors in Jenkins

This document summarizes the complete steps for setting up automatic tracking of code warning authors in Jenkins. With this configuration, you can easily identify who introduced warnings in your code, thereby improving code quality and team accountability.

## Prerequisites

- Jenkins server installed and running
- Source code hosted in a Git repository (e.g., GitHub)
- Project using CMake build system

## Step 1: Install Required Plugins

1. Log in to the Jenkins management interface
2. Navigate to "Manage Jenkins" > "Manage Plugins" > "Available" tab
3. Search for and install the following plugins:
   - **Git Plugin**: Provides Git integration
   - **Pipeline Plugin**: Supports creating Pipeline jobs
   - **Warnings Next Generation Plugin**: For collecting and analyzing compiler warnings
   - **Forensics API Plugin**: Provides API for code analysis and author tracking
   - **Git Forensics Plugin**: Offers more detailed code analysis for Git repositories
4. Restart Jenkins after installation if necessary

## Step 2: Create a Multibranch Pipeline Project

1. On the Jenkins homepage, click "New Item"
2. Enter a project name (e.g., "MyProject-Warnings-Tracker")
3. Select "Multibranch Pipeline" type
4. Click "OK"

## Step 3: Configure the Multibranch Pipeline

1. In the "Branch Sources" section:
   - Select "Git"
   - Enter your Git repository URL (e.g., `https://github.com/yourusername/yourrepo.git`)
   - Add credentials if needed
   - Configure branch filters (optional)

2. In the "Build Configuration" section:
   - Select "by Jenkinsfile"
   - Keep "Script Path" as the default value `Jenkinsfile`

3. In the "Scan Multibranch Pipeline Triggers" section:
   - Check "Periodically if not otherwise run"
   - Set a scan interval (e.g., "1 hour")

4. Click "Save"

## Step 4: Create the Jenkinsfile

Create a file named `Jenkinsfile` in the root directory of your Git repository with the following content:

```groovy
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
                    // Get current commit information
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
```

## Step 5: Commit and Push the Jenkinsfile

```bash
git add Jenkinsfile
git commit -m "Add Jenkinsfile with warnings tracking configuration"
git push
```

## Step 6: Trigger the Build

1. Return to the Multibranch Pipeline project in Jenkins
2. Click "Scan Multibranch Pipeline Now" to scan the repository
3. Wait for the scan to complete; Jenkins will automatically detect your branches
4. Click on the detected branch (e.g., "master" or "main")
5. Click "Build Now" to trigger a build

## Step 7: View Warnings and Authors

After the build completes:

1. In the branch's Pipeline page, click on the "Warnings" link in the left navigation bar
2. On the warnings page, you can see all detected warnings
3. Click the "SCM Blames" tab to view warning author information
4. You can see for each warning:
   - File path and line number
   - Warning message
   - Developer who introduced the warning
   - Commit information that introduced the warning

## Configuration Explanation

### Pipeline Script Analysis

- **checkout scm**: Checks out the source code
- **bat commands**: Executes CMake build with high warning levels enabled
- **Git Info stage**: Collects information about the current commit, including commit ID, author, and email
- **recordIssues**: Collects and analyzes build warnings
  - **enabledForFailure: true**: Collects warnings even if the build fails
  - **tools: [msBuild()]**: Uses MSBuild tool to parse warnings
  - **skipBlames: false**: Enables blame functionality to track warning authors

### Warning Analysis Configuration

The Warnings Next Generation plugin automatically parses warning information from build logs and associates it with Git history records to determine who introduced each warning.

## Troubleshooting

If you encounter issues during setup:

1. **Plugin compatibility issues**: Ensure all plugins are up-to-date and compatible with each other
2. **Pipeline syntax errors**: Use Jenkins' "Pipeline Syntax" generator to check syntax
3. **Git access issues**: Ensure Jenkins has permission to access your Git repository
4. **Build failures**: Check build logs to identify specific errors

## Extended Functionality

You can further extend this configuration:

1. **Add quality gates**: Set warning count thresholds that mark builds as unstable or failed when exceeded
2. **Add trend charts**: Display how warning counts change over time
3. **Integrate notifications**: Send warning author information to Slack, email, or other notification channels
4. **Add more static analysis tools**: Such as Clang-Tidy, Cppcheck, etc.

## Conclusion

By following these steps, you have successfully configured Jenkins to automatically track code warning authors. This will help your team better manage code quality, clarify responsibility attribution, and promote continuous improvement.

---

Good luck with your project development!