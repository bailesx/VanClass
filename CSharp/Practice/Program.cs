// 네임 스페이스 시스템 사용
using System;
// Generic Collection을 정의하는 Class와 Interface
using System.Collections.Generic;
// LINQ를 사용하는 쿼리를 지원하는 Class와 Interface
using System.Linq;
using System.Text;

// namespace : 구조체, 클래스, 델리케이트, 인터페이스 등을 그룹화한 것/
namespace Practice
{
    // class : 크게 field와 method로 분류되며 각 정적인 상태와 동적인 기능을 표현함
    //         이는 독립적으로 존재할 수 있는 최소 단위이자 C#을 구성하는 기본 단위
    //         하나의 클래스를 통해 여러 개의 객체 생성 가능
    class Program
    {
        // Main method : 프로그램의 최초 진입점
        static void Main(string[] args)
        {
            // 비개행 출력
            Console.Write("Hello World!");
            // 자동 개행 출력
            Console.WriteLine("Hello World!");

            // 정수 자료형
            byte a = 255;
            sbyte b = 127;
            short c = 32767;
            ushort d = 65535;
            int e = int.MaxValue;
            uint f = 4294967295;
            long g = 922337203685477508;
            ulong h = 18446744073709551615;
            // 실수 자료형
            float i = 0.7f;
            double j = 0.16;
            decimal k = 0.29m;
            // 논리 자료형
            bool l = false;
            // 문자 자료형
            char m = '\0';
            string n = "hello world";

            // 정수(int)나 날짜(datetime) 등의 자료형에 NULL을 가질 수 있도록 해주는 기능
            int? o = null;
            // ?는 실제로 아래와 같이 Nullable<T> 형태로 기능함
            Nullable<int> p = null;

            // 상속의 개념. 모든 자료형의 최상위 클래스인 object로부터 상속을 받는 형태로 어떤 자료형에도 사용 가능
            // 단, value type과 reference type을 오가는 형 변환이 이루어지는 경우,
            // 박싱(boxing : 값형식 -> 참조형식), 언박싱(unboxing : 참조형식 -> 값형식) 과정이 일어나며 성능상 영향을 미칠 수 있다
            object q = 2147483647;
            q = "hi";
            // compile time에 타입이 결정되는 타입 유추 형식의 자료형 
            var r = 2147483647;
            // 데이터 형식을 동적으로 변경할 수 있는 자료형
            // run time에 타입이 결정되는 타입 유추 형식의 자료형으로, 잘못된 형태가 되더라도 compile에서는 문제가 드러나지 않으므로 주의 요망
            dynamic s = 2147483647;
            s = "hello";

            // anonymous type
            // 읽기 전용으로 갱신 불가하며, 공식적으로 class를 사용할 필요 없이 임시로 만들어 사용할 때 유용함
            object t = new { Name = "Felipe", Age = 20 };
            var u = new { Name = "Felipe", Age = 20 };
            dynamic v = new { Name = "Felipe", Age = 20 };

            // 이하 구조체(struct), 열거형(enum)이 존재

            // 변수의 콘솔 출력 방식 - 배열 형식
            Console.WriteLine("정수 = {0}, 실수 = {1}, 문자열 = {2}",e,j,n);

            // 정수/실수 -> 문자열 -> 정수/실수
            int baseA = 12345;
            string changeA = baseA.ToString();
            int changeB = int.Parse(changeA);

            // 배열 선언과 반복문 foreach
            int[] array1 = {1, 2, 3, 4, 5};
            int[] array2 = new int[] { 1, 2, 3, 4, 5 };
            int[] array3 = new int[5];
            foreach (int count in array1)
            {
                Console.WriteLine("{0} ", count);
            }

            // Method와 call by reference
            int swapA = 59;
            int swapB = 36;

            // 선언된 함수가 static형이 아니라면 클래스의 객체를 이용해서 함수를 사용해야 함
            Program pro = new Program();
            pro.SwapA(ref swapA, ref swapB);

            // 선언된 함수가 static형이라면 함수 그대로 사용 가능
            SwapB(ref swapA, ref swapB);
        }
        
        void SwapA(ref int swapA, ref int swapB)
        {
            int temp = swapA;
            swapA = swapB;
            swapB = temp;
        }

        static void SwapB(ref int swapA, ref int swapB)
        {
            int temp = swapA;
            swapA = swapB;
            swapB = temp;
        }
    }
}
