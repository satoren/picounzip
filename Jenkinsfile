node('linux') {
 stage 'Test'
 checkout scm
 sh 'pip install --user backports.tempfile'
 sh 'cd tests && python create_large_testdata.py'
 sh 'python test_runner.py'
}
