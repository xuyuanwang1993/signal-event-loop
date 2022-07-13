#!groovy
properties([
    parameters([
        choice(name: 'platform', choices: ['linux', 'android'], description: 'choose build platform'),
        choice(name: 'arch', choices: ['3288', '3328', '3399', '3568'], description: 'specify arch')
    ])
])
pipeline{
    agent {
        node {
            label "firmware"
        }
    }
    environment {
        package_name="aimy-eventloop"
    }
    options {
        timestamps()
        disableConcurrentBuilds()
    }
    stages{
        stage('prepare'){
            steps{
                echo "prepare"
                script{
                    sh 'printenv | sort'
                    sh "git submodule update --init --recursive"
                    install_path="tools/${env.package_name}"
                    target_platform="${platform}"
                    taget_arch="${arch}"
                    output_path="${env.WORKSPACE}/output"
                    sh "mkdir -p ${firmware_package_path}/${install_path}"
                    sh "mkdir -p ${output_path}"
                    echo "install_path=${firmware_package_path}/${install_path}"
                    
                    echo "target_platform=${target_platform}"
                    echo "taget_arch=${taget_arch}"
                    echo "output_path=${output_path}"
                }
            }
        }
        stage('build'){
            steps{
                echo "build"
                script{
                    sh "bash compile/prod-build.sh ${taget_arch} ${target_platform} ${firmware_package_path}/${install_path} ${output_path}"
                }
            }
        }
    }
    post {
        success {
                echo 'anything success!'
                script {
                    currentBuild.description = "${target_platform}-${taget_arch}<td><a href=\"http://${sdk_download_service_url}/${install_path}/${target_platform}-${taget_arch}/latest\" target=_blank>resource page</a></td>"  
                }
        }
        failure {
            echo 'pipeline failed'
        }
    }
}
