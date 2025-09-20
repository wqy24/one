(define (tokens port)

 (define (read-when pred? acc port)
  (let [[next (peek-char port)]]
   (if (and (not (eof-object? next)) (pred? next))
    (read-when pred? (string-append acc (string (read-char port))) port)
    acc)))

 (define (read-number)
  (let [[rat #t]
        [dot #t]
        [e   #t]]
   (lambda (c)
    (cond
     [(char-numeric? c) #t]
     [(and (eqv? c #\.) rat dot e)
      (set! dot #f)
      #t]
     [(and (or (eqv? c #\e) (eqv? c #\E)) e rat)
      (set! e #f)
      #t]
     [(and (eqv? c #\/) rat dot e)
      (set! rat #f)
      #t]
     [else #f]))))

 (define (a-token)
  (let [[next-ch (read-char port)]
        [token-stream (lambda (v) (cons v (delay (a-token))))]]
   (cond
    [(eof-object? next-ch) '()]
    [(char-whitespace? next-ch) (a-token)]
    [(eqv? next-ch #\[) (token-stream 'lbrk)]
    [(eqv? next-ch #\]) (token-stream 'rbrk)]
    [(eqv? next-ch #\() (token-stream 'lprn)]
    [(eqv? next-ch #\)) (token-stream 'rprn)]
    [(eqv? next-ch #\,) (token-stream 'comma)]
    [(eqv? next-ch #\&) (token-stream 'et)]
    [(eqv? next-ch #\:) (token-stream 'colon)]
    [(eqv? next-ch #\>) (token-stream 'arrow)]
    [(or (eqv? next-ch #\-) (char-numeric? next-ch))
     (token-stream (read-when (read-number) (string next-ch) port))]
    [else (error "Not Implemented yet") #| It will raise an error - in raising an error |#])))

 (a-token))

; Use pairs and null to express AST so that we can express the AST in 1 the Programming language

(define-syntax ast-call
 (syntax-rules ()
  [(_ fn arg) `(((,fn . ,arg) . ()) . (() . ()))]))

(define-syntax ast-fn
 (syntax-rules ()
  [(_ ast) `((() . ,ast) . (() . ()))]))

(define-syntax ast-param
 (syntax-rules ()
  [(_ layer) `((() . ()) . (,layer . ()))]))

(define-syntax ast-literal
 (syntax-rules ()
  [(_ value) `((() . ()) . (() . ,value))]))
