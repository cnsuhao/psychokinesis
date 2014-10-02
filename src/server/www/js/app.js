angular.module('psychokinesis', ['ngRoute'])
		.controller('DefaultController', function($scope, $route, $routeParams, $location) {
			$scope.download = download_app;
		})
		.controller('SuggestController', function($scope, $route, $routeParams, $location) {
			$scope.isUnchanged = function(suggestion) {
				return suggestion == undefined || 
					   suggestion.type == undefined ||
					   suggestion.content == undefined || suggestion.content == '';
			};
			
			$scope.suggest = function(suggestion) {
				$("#suggest_button").val('请稍等');
				$("#suggest_button").attr("disabled", true);
				
				$.post("nodejs/suggest", suggestion, function(data, status) {
					$("#suggest_button").val('确定');
					$("#suggest_button").attr("disabled", false);
					
					if (status != 'success') {
						message_box('错误', '网络故障：' + status);
						return;
					}
					
					if (data.error_code) {
						message_box('错误', convert_error_string(data.error_code));
						return;
					}
					
					$location.path('/success').search({type: 'thank_suggest'});
					$scope.$apply();
				}, "json");
			};
		})
		.controller('JoinController', function($scope, $route, $routeParams, $location) {
			$scope.isUnchanged = function(person) {
				return person == undefined || 
					   person.interest == undefined ||
					   person.email == undefined || person.email == '';
			};
			
			$scope.join = function(person) {
				$("#join_submit_button").val('请稍等');
				$("#join_submit_button").attr("disabled", true);
				
				$.post("nodejs/join", person, function(data, status) {
					$("#join_submit_button").val('提交');
					$("#join_submit_button").attr("disabled", false);
					
					if (status != 'success') {
						message_box('错误', '网络故障：' + status);
						return;
					}
					
					if (data.error_code) {
						message_box('错误', convert_error_string(data.error_code));
						return;
					}
					
					$location.path('/success').search({type: 'join_success'});
					$scope.$apply();
				}, "json");
			};
		})
		.controller('SuccessController', function($scope, $route, $routeParams, $location) {
			if ($routeParams.type == 'thank_suggest') {
				$scope.prompt_img = 'images/thank.gif';
				$scope.prompt_text = '谢谢您的反馈！我们将尽快改进。';
			} else if ($routeParams.type == 'join_success') {
				$scope.prompt_img = 'images/right.jpg';
				$scope.prompt_text = '提交成功！请耐心等待，我们将在三天内与你联系。';
			} else if ($routeParams.type == 'register_success') {
				$scope.prompt_img = 'images/right.jpg';
				$scope.prompt_text = '注册成功！登录客户端开启专属您的Psychokinesis吧！';
			} else {
				$location.path('/');
			}
		})
		.controller('RegisterController', function($scope, $route, $routeParams, $location) {
			$scope.invalidAccount = /^[^'";&|\s]+$/;
					
			$scope.isUnchanged = function(info) {
				return info.account == undefined || info.account == '' ||
					   info.password == undefined || info.password == '' ||
					   info.email == undefined || info.email == '';
			};
					
			$scope.register = function(info) {
				$("#register_button").val('请稍等');
				$("#register_button").attr("disabled", true);
					
				$.post("nodejs/register-by-app", info)
					.done(function(data) {
						if (data.error_code) {
							message_box('错误', convert_error_string(data.error_code));
							$("#register_button").val('注册账号');
							$("#register_button").attr("disabled", false);
							return;
						}
				  
						$location.path('/success').search({type: 'register_success'});
						$scope.$apply();
					})
					.fail(function() {
						message_box('错误', '网络故障！请稍后重试。');
						$("#register_button").val('注册账号');
						$("#register_button").attr("disabled", false);
						return;
					});
			};
		})
		.config(function($routeProvider, $locationProvider) {
			$routeProvider
				.when('/', {
					templateUrl: 'default.html',
					controller: 'DefaultController'
				})
				.when('/suggest', {
					templateUrl: 'suggest.html',
					controller: 'SuggestController'
				})
				.when('/join', {
					templateUrl: 'join.html',
					controller: 'JoinController'
				})
				.when('/join-form', {
					templateUrl: 'join_form.html',
					controller: 'JoinController'
				})
				.when('/success', {
					templateUrl: 'success.html',
					controller: 'SuccessController'
				})
				.when('/register', {
					templateUrl: 'register.html',
					controller: 'RegisterController'
				})
				.otherwise({
					redirectTo: '/'
				});
						
			$locationProvider.html5Mode(true);
		});