#include <stdio.h>
int main() {
	int n;
	scanf("%d", &n);

	if (isPalindrome(n)) {
		printf("Y\n");
	} else {
		printf("N\n");
	}
	return 0;
}

int isPalindrome(int n){
	char str[5];
	int i=0;
	while(n>0){
		if(n>0){
			str[i++]=n%10;
			n/=10;
		}
	}
	for(int j=0;j<strlen(str);j++){
		if(str[strlen(str)-1-j]!=str[j]) return 0;
	}
	return 1;
}
