#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <apr.h>
#include <apr_general.h>
#include <apr_hash.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <dirent.h>
#include <time.h>

/*--- do modify By Miyasaka */
#define PROG_ENV "/home/miyasaka/mp3/info_mp3.sh"
#define PROG "info_mp3.sh"
#define UPLOAD_FILE_DIR "/home/miyasaka/mp3/Music/"
#define UPLOAD_FILE_NAME "end.txt"
#define MAX_CHILDREN 1         //set child process numbers
#define CHILD_SLEEP_TIME 60
#define PID_FILE "/var/tmp/prefork.pid"

void kill_all_children(int);
void signal_handler(int);
void write_pid();
void daemonize();
int  file_search(void);


// 子プロセスの管理にハッシュテーブルを使う
static apr_pool_t* pool;
static apr_hash_t* hChildren;

int
file_search(){
	DIR *dir;
	struct dirent *dp;
	// char wk_file_name[256];

	// strcpy(wk_file_name,UPLOAD_FILE_DIR);
	// strcat(wk_file_name,UPLOAD_FILE_NAME);

	dir = opendir(UPLOAD_FILE_DIR);
	while((dp = readdir(dir)) != NULL){
		if(dp->d_name[0] != '.'){
			closedir(dir);
			return(1);
		}
	}
	closedir(dir);
	return(0);
}

int
main(int argc, char **argv){
  	int semid;
  	struct sembuf sembuff;

	// セマフォの初期化ファイルアクセスは１プロセス単位
	semid = semget(IPC_PRIVATE, 1, S_IRUSR|S_IWUSR);   
	if ( semid == -1 )   
        	exit(1);   
  
	if ( semctl(semid, 0, SETVAL, 1) != 0 )   
        	exit(0); 

	// デーモン化
	daemonize();

	//kill しやすいように pid ファイルの作成
	write_pid();

	// SIGTERM ですべての子プロセスを殺すようにシグナルハンドラを設定
	signal(SIGTERM, kill_all_children);


	// ハッシュの初期化
	apr_initialize();
  	apr_pool_create(&pool, NULL);
  	hChildren = apr_hash_make(pool);
  
  	//親プロセスのループ
  	while(1){
		while( apr_hash_count(hChildren) >= MAX_CHILDREN ){
	  	int status;
	  	pid_t child_pid = wait( &status ); //子プロセスが死ぬまで待つ
	  		//死んだ子プロセスをハッシュテーブルから削除
	  		(APR_DECLARE(void))apr_hash_set(hChildren, &child_pid, sizeof(child_pid), (const void *)NULL);
		}

		pid_t *pid = apr_palloc(pool, sizeof(pid_t));
		*pid = fork(); //フォーク

		// printf("fork ppid[%d]:pid[%d]\n",getppid(),getpid());
		if(*pid==0){
		  signal(SIGTERM, signal_handler);
          break; //子プロセスだったら、ループから抜ける
		}

#ifdef MIYASAKA
		//子プロセスをハッシュテーブルに追加
		apr_hash_set(hChildren, pid, sizeof(pid_t), (const void *)"1");
#else
		//子プロセスをハッシュテーブルに追加
		apr_hash_set(hChildren, pid, sizeof(pid_t), 1);
#endif
		sleep(CHILD_SLEEP_TIME);
  	}
  
  	while(1){
		//子プロセスの処理
        	// ファイルアクセスは１プロセス単位
        	sembuff.sem_num = 0;
        	sembuff.sem_op = -1;
        	sembuff.sem_flg = SEM_UNDO;
		semop(semid,&sembuff,1);

        	//file search
		if(file_search()){
			//call MP3 converter shell
			execl(PROG_ENV,PROG,NULL);
			perror("Failed execl:");
		}
		sleep(CHILD_SLEEP_TIME);

       		sembuff.sem_num = 0;
       		sembuff.sem_op = 1;
       		sembuff.sem_flg = SEM_UNDO;
		semop(semid,&sembuff,1);
	}
}

void
kill_all_children(int sig){
  	apr_hash_index_t *hi;
  	apr_ssize_t klen;
  	pid_t *child_pid_ptr;
  	int val;

  	for( hi=apr_hash_first(pool, hChildren); hi; hi=apr_hash_next(hi) ){
#ifdef MIYASAKA
		apr_hash_this(hi, (const void **)&child_pid_ptr, &klen, (void **)&val);
#else
		apr_hash_this(hi, &child_pid_ptr, &klen, &val);
#endif
		int ret = kill(*child_pid_ptr, SIGTERM);
  	}
  	exit(0);
}

void
signal_handler(int sig){
  	exit(0);
}

void
write_pid(){
  	FILE *fp;
  	fp = fopen(PID_FILE, "w");
  	fprintf(fp, "%d", getpid());
  	fclose(fp);
}

void
daemonize(){
	 pid_t pid, sid;
  
  	/* カレントディレクトリを / に変更 */
  	if( chdir("/") < 0 )	exit(EXIT_FAILURE);
  
  	/* 子プロセスの開始 */
  	pid = fork();
  	if( pid < 0 ) exit(EXIT_FAILURE);
  
  	/* 親プロセスを終了 */
  	if( pid > 0 ) exit(EXIT_SUCCESS);
  
  	/* 新しいセッションを作成 */
  	sid = setsid();
  	if( sid < 0 ) exit(EXIT_FAILURE);
  
  	/* ファイル作成マスクをリセット */
  	umask(0);
  
  	/* 標準入力，標準出力，標準エラー出力を閉じる */
#ifndef DEBUG
  	close(STDIN_FILENO);
  	close(STDOUT_FILENO);
  	close(STDERR_FILENO);
#endif
  return;
}

