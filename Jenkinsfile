node('linux') {
 stage 'Test'
 checkout scm
 sh 'cd tests && python create_large_testdata.py'
 sh 'python test_runner.py'
}
