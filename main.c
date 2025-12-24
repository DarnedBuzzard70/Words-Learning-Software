#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <Windows.h>

#define MAX_WORDS 1000  // 最大单词存储数量
#define MAX_WORD_LEN 50 // 英文单词最大长度
#define MAX_MEAN_LEN 100// 中文释义最大长度
#define WORD_FILE_NAME "word_book.txt" // 默认单词存储文件名

// 单词结构体定义
typedef struct {
    char english[MAX_WORD_LEN];   // 英文单词
    char chinese[MAX_MEAN_LEN];   // 中文释义
    int error_count;              // 错误次数
    int correct_streak;           // 连续答对次数
    int is_mastered;              // 是否已掌握（0-未掌握，1-已掌握）
} Word;

Word word_book[MAX_WORDS];  // 单词本
int word_count = 0;         // 当前单词总数

// 函数声明
void add_word();
int is_word_exist(const char *eng);
void take_test();
int get_word_weight(int index);
void query_words();
void delete_word();
void show_menu();
void init_word_book();
void save_words_to_file();    // 新增：保存单词到文件
void load_words_from_file();  // 新增：从文件读取单词
void load_words_from_file_init(); //手动添加 第一次自动读取 

// 初始化单词本
void init_word_book() {
    word_count = 0;
    memset(word_book, 0, sizeof(word_book));
    srand((unsigned int)time(NULL)); // 初始化随机数种子
}

// 添加单词（自动去重）
void add_word() {
    if (word_count >= MAX_WORDS) {
        printf("单词本已满，无法添加新单词！\n");
        return;
    }

    char eng[MAX_WORD_LEN];
    char mean[MAX_MEAN_LEN];

    printf("请输入英文单词：");
    scanf("%s", eng);
    getchar(); // 吸收换行符

    // 检查单词是否已存在
    if (is_word_exist(eng) != -1) {
        printf("该单词已收录，无需重复添加\n");
        return;
    }

    printf("请输入中文释义：");
    gets(mean); // 读取带空格的中文释义

    // 存入单词本
    strcpy(word_book[word_count].english, eng);
    strcpy(word_book[word_count].chinese, mean);
    word_book[word_count].error_count = 0;
    word_book[word_count].correct_streak = 0;
    word_book[word_count].is_mastered = 0;
    word_count++;

    printf("单词添加成功！\n");
}

// 检查单词是否存在，存在返回索引，不存在返回-1
int is_word_exist(const char *eng) {
    for (int i = 0; i < word_count; i++) {
        if (strcmp(word_book[i].english, eng) == 0) {
            return i;
        }
    }
    return -1;
}

// 智能抽取测试（优先重点单词）
void take_test() {
    if (word_count == 0) {
        printf("单词本中暂无单词，无法进行测试！\n");
        return;
    }

    printf("========== 智能单词测试 ==========\n");
    printf("规则：优先测试错误3次及以上的重点单词，输入中文释义对应的英文单词\n");

    // 选择待测试单词（带权重抽取）
    int selected_index = -1;
    int total_weight = 0;

    // 计算总权重
    for (int i = 0; i < word_count; i++) {
        total_weight += get_word_weight(i);
    }

    // 随机抽取（按权重分配概率）
    int random_num = rand() % total_weight;
    int weight_sum = 0;
    for (int i = 0; i < word_count; i++) {
        weight_sum += get_word_weight(i);
        if (random_num < weight_sum) {
            selected_index = i;
            break;
        }
    }

    if (selected_index == -1) {
        printf("抽取单词失败！\n");
        return;
    }

    // 开始测试
    char input_word[MAX_WORD_LEN];
    printf("中文释义：%s\n", word_book[selected_index].chinese);
    printf("请输入英文单词：");
    scanf("%s", input_word);

    // 判断对错
    if (strcmp(input_word, word_book[selected_index].english) == 0) {
//    	system("color F2"); 
        printf("回答正确！\n");
//        system("color F0"); 
        // 更新连续答对次数和错误次数
        word_book[selected_index].correct_streak++;
        word_book[selected_index].error_count = (word_book[selected_index].error_count > 0) ? (word_book[selected_index].error_count - 1) : 0;

        // 连续答对3次，标记为已掌握
        if (word_book[selected_index].correct_streak >= 3) {
            word_book[selected_index].is_mastered = 1;
            printf("该单词已掌握，降低复习频率！\n");
        }
    } else {
//    	system("color F4"); 
        printf("回答错误！正确答案：%s\n", word_book[selected_index].english);
//        system("color F0"); 
        // 更新错误次数和连续答对次数
        word_book[selected_index].error_count++;
        word_book[selected_index].correct_streak = 0;
        word_book[selected_index].is_mastered = 0; // 答错后取消已掌握标记
    }

    printf("==================================\n");
}

// 获取单词抽取权重（重点单词权重2，普通单词权重1）
int get_word_weight(int index) {
    // 错误3次及以上为重点单词，权重2；普通单词权重1；已掌握单词权重1（降低优先级但仍可抽取）
    if (word_book[index].error_count >= 3) {
        return 2;
    } else {
        return 1;
    }
}

// 智能查询单词（按掌握程度筛选）
void query_words() {
    if (word_count == 0) {
//    	system("color F4"); 
        printf("单词本中暂无单词！\n");
//        system("color F0"); 
        return;
    }

    int choice;
    printf("========== 智能单词查询 ==========\n");
    printf("1 - 显示重点单词（错误3次及以上）\n");
    printf("2 - 显示已掌握单词\n");
    printf("3 - 显示所有单词\n");
    printf("请输入查询选项：");
    scanf("%d", &choice);

    printf("========== 查询结果 ==========\n");
    printf("英文\t\t中文\t\t掌握状态\n");
    printf("-------------------------------\n");

    for (int i = 0; i < word_count; i++) {
        int is_match = 0;
        char status[20];

        // 判断掌握状态
        if (word_book[i].error_count >= 3) {
//        	system("color F4"); 
            strcpy(status, "重点单词（未掌握）");
//            system("color F0"); 
        } else if (word_book[i].is_mastered == 1) {
//        	system("color F2"); 
            strcpy(status, "已掌握");
//            system("color F0"); 
        } else {
            strcpy(status, "普通单词（未掌握）");
        }

        // 按选项筛选
        switch (choice) {
            case 1:
                if (word_book[i].error_count >= 3) {
                    is_match = 1;
                }
                break;
            case 2:
                if (word_book[i].is_mastered == 1) {
                    is_match = 1;
                }
                break;
            case 3:
                is_match = 1;
                break;
            default:
//            	system("color F4"); 
                printf("无效选项！\n");
//                system("color F0"); 
                return;
        }

        // 输出匹配结果
        if (is_match) {
            printf("%-12s\t%-15s\t%s\n", word_book[i].english, word_book[i].chinese, status);
        }
    }

    printf("==================================\n");
}

// 个性化删除单词（带确认提示）
void delete_word() {
    if (word_count == 0) {
//    	system("color F4"); 
        printf("单词本中暂无单词，无法删除！\n");
//        system("color F0");
        return;
    }

    char eng[MAX_WORD_LEN];
    printf("请输入要删除的英文单词：");
    scanf("%s", eng);

    int index = is_word_exist(eng);
    if (index == -1) {
        printf("该单词不存在于单词本中！\n");
        return;
    }

    // 提示确认删除
    int error_num = word_book[index].error_count;
    int confirm;
//    system("color F4"); 
    printf("该单词您错误次数为%d次，删除后将失去复习记录，是否确认？（1-是，0-否）：", error_num);
//    system("color F0"); 
	scanf("%d", &confirm);

    if (confirm != 1) {
        printf("已取消删除操作！\n");
        return;
    }

    // 执行删除（后面的单词前移覆盖）
    for (int i = index; i < word_count - 1; i++) {
        word_book[i] = word_book[i + 1];
    }
    word_count--;
    memset(&word_book[word_count], 0, sizeof(Word)); // 清空最后一个位置
//	system("color F2"); 
    printf("单词删除成功！\n");
//    system("color F0"); 
}

// 新增：将单词本保存到文件
void save_words_to_file() {
    // 打开文件（写入模式，不存在则创建，存在则覆盖）
    FILE *fp = fopen(WORD_FILE_NAME, "w");
    if (fp == NULL) {
//    	system("color F4"); 
        printf("打开文件失败，无法保存单词本！\n");
//        system("color F0"); 
        return;
    }

    // 写入单词总数（便于读取时识别）
    fprintf(fp, "%d\n", word_count);

    // 循环写入每个单词的完整信息
    for (int i = 0; i < word_count; i++) {
        fprintf(fp, "%s\n", word_book[i].english);
        fprintf(fp, "%s\n", word_book[i].chinese);
        fprintf(fp, "%d\n", word_book[i].error_count);
        fprintf(fp, "%d\n", word_book[i].correct_streak);
        fprintf(fp, "%d\n", word_book[i].is_mastered);
    }

    // 关闭文件
    fclose(fp);
//    system("color F2"); 
    printf("单词本已成功保存到文件：%s\n", WORD_FILE_NAME);
//    system("color F0"); 
}

// 新增：从文件读取单词本
void load_words_from_file() {
    // 打开文件（读取模式）
    FILE *fp = fopen(WORD_FILE_NAME, "r");
    if (fp == NULL) {
//    	system("color F4"); 
        printf("未找到单词本文件：%s，无法读取！\n", WORD_FILE_NAME);
//        system("color F0"); 
        return;
    }

    // 先清空原有单词本，避免数据冲突
    init_word_book();

    // 读取单词总数
    int read_count = 0;
    fscanf(fp, "%d", &read_count);
    fgetc(fp);
//    getchar(); // 吸收换行符

    // 校验单词数量是否超出最大值
    if (read_count > MAX_WORDS) {
//    	system("color F4"); 
        printf("文件中单词数量超出上限，仅读取前%d个单词！\n", MAX_WORDS);
//        system("color F0"); 
        read_count = MAX_WORDS;
    }

    // 循环读取每个单词的信息
    for (int i = 0; i < read_count; i++) {
        // 读取英文单词
        fgets(word_book[i].english, MAX_WORD_LEN, fp);
        // 去除fgets读取到的换行符
        word_book[i].english[strcspn(word_book[i].english, "\n")] = '\0';

        // 读取中文释义
        fgets(word_book[i].chinese, MAX_MEAN_LEN, fp);
        word_book[i].chinese[strcspn(word_book[i].chinese, "\n")] = '\0';

        // 读取错误次数、连续答对次数、是否已掌握
        fscanf(fp, "%d", &word_book[i].error_count);
        fscanf(fp, "%d", &word_book[i].correct_streak);
        fscanf(fp, "%d", &word_book[i].is_mastered);
//        getchar(); // 吸收换行符
		fgetc(fp);
        word_count++;
    }

    // 关闭文件
    fclose(fp);
//    system("color F2"); 
    printf("成功从文件：%s 读取 %d 个单词！\n", WORD_FILE_NAME, word_count);
//    system("color F0"); 
}
void load_words_from_file_init() {
    FILE *fp = fopen(WORD_FILE_NAME, "r");
    if (fp == NULL) {
        return;
    }

    // 先清空原有单词本，避免数据冲突
    init_word_book();

    // 读取单词总数
    int read_count = 0;
    fscanf(fp, "%d", &read_count);
    fgetc(fp); 
//    getchar(); // 吸收换行符

    // 校验单词数量是否超出最大值
    if (read_count > MAX_WORDS) {
//    	system("color F4"); 
        printf("文件中单词数量超出上限，仅读取前%d个单词！\n", MAX_WORDS);
//        system("color F0"); 
        read_count = MAX_WORDS;
    }

    // 循环读取每个单词的信息
    for (int i = 0; i < read_count; i++) {
        // 读取英文单词
        fgets(word_book[i].english, MAX_WORD_LEN, fp);
        // 去除fgets读取到的换行符
        word_book[i].english[strcspn(word_book[i].english, "\n")] = '\0';

        // 读取中文释义
        fgets(word_book[i].chinese, MAX_MEAN_LEN, fp);
        word_book[i].chinese[strcspn(word_book[i].chinese, "\n")] = '\0';

        // 读取错误次数、连续答对次数、是否已掌握
        fscanf(fp, "%d", &word_book[i].error_count);
        fscanf(fp, "%d", &word_book[i].correct_streak);
        fscanf(fp, "%d", &word_book[i].is_mastered);
        fgetc(fp); 
//        getchar(); // 吸收换行符

        word_count++;
    }

    // 关闭文件
    fclose(fp);
}
// 显示主菜单（新增文件操作选项）
void show_menu() {
    printf("\n========== 单词学习辅助工具 ==========\n");
    printf("1 - 添加新单词\n");
    printf("2 - 智能单词测试（优先重点单词）\n");
    printf("3 - 按掌握程度查询单词\n");
    printf("4 - 个性化删除单词\n");
    printf("5 - 保存单词本到文件\n");  // 新增菜单选项
    printf("6 - 从文件读取单词本\n");  // 新增菜单选项
    printf("0 - 退出程序\n");
    printf("=====================================\n");
    printf("请输入功能选项：");
}

// 主函数
int main() {
	system("color F0");
    init_word_book();
    int choice;
	load_words_from_file_init();
    while (1) {
        show_menu();
        scanf("%d", &choice);
		int zflag=0;
        switch (choice) {
            case 1:
                add_word();
                break;
            case 2:
                take_test();
                zflag=1;
                break;
            case 3:
                query_words();
                break;
            case 4:
                delete_word();
                break;
            case 5:
                save_words_to_file();  // 调用保存文件函数
                break;
            case 6:
                load_words_from_file(); // 调用读取文件函数
                break;
            case 0:
    			srand(time(0));
				int sd=rand()%10;
				if(sd==1){
					printf("The more you practice English, the more fluent you will become. No pain, no gain.\n");
				}else if(sd==2){
					printf("Every word you memorize, every sentence you read, is a step closer to mastering English.\n");
				}else if(sd==3){
					printf("Don't be afraid of making mistakes when speaking English. Mistakes are just proof that you are trying.\n");
				}else if(sd==4){
					printf("Consistency beats intensity. Even 15 minutes of English learning a day will lead to big changes over time.\n");
				}else if(sd==5){
					printf("English is not a subject to be learned, but a skill to be practiced. Keep speaking, keep writing, keep improving.\n");
				}else if(sd==6){
					printf("Today's effort is tomorrow's ability. Never give up on your English learning journey.\n");
				}else if(sd==7){
					printf("A single vocabulary word may seem small, but a thousand words will build a bridge to a new world.\n");
				}else if(sd==8){
					printf("The best time to start learning English was yesterday. The second best time is now.\n");
				}else if(sd==9){
					printf("Difficulties in English learning are temporary, but the benefits of mastering it are lifelong.\n");
				}else if(sd==0){
					printf("You don't have to be perfect to speak English—you just have to be brave enough to start.\n");
				}
                printf("感谢使用单词学习辅助工具，再见！(按 Enter 关闭程序)\n");
                getchar();
                exit(0);
            default:
                printf("无效的选项，请重新输入！\n");
                break;
        }

        // 暂停一下，方便查看结果
        if(zflag==0){
        	printf("\n按回车键继续...");
		}
        getchar();
    }

    return 0;
}
